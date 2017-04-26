/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "https_request.h"
#include "http_response_parser.h"

#define TLS_CUNSTOM "Arduino TLS client"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// SSL callback
/**
 * Receive callback for mbed TLS
 */
static int ssl_recv(void *ctx, unsigned char *buf, size_t len) 
{
    int recv = -1;
    TCPSocket *socket = static_cast<TCPSocket *>(ctx);
    for (int i = 0; i < 5; ++i)
    {
        recv = socket->recv(buf, len);
        if (recv != 0) break;
        wait_ms(500);
    }

    if (NSAPI_ERROR_WOULD_BLOCK == recv)
    {
        return MBEDTLS_ERR_SSL_WANT_READ;
    }
    else if (recv <= 0)
    {
        return -1;
    }
    else
    {
        return recv;
    }
}

/**
 * Send callback for mbed TLS
 */
static int ssl_send(void *ctx, const unsigned char *buf, size_t len)
{
   int size = -1;
    TCPSocket *socket = static_cast<TCPSocket *>(ctx);
    for (int i = 0; i < 5; ++i)
    {
        size = socket->send(buf, len);
        if (size != 0) break;
        wait_ms(500);
    }

    if(NSAPI_ERROR_WOULD_BLOCK == size)
    {
        return len;
    }
    else if (size <= 0)
    {
        return -1;
    }
    else
    {
        return size;
    }
}

#if DEBUG_LEVEL > 0
static void my_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    char tmp[32];
    const char *p, *basename;
    
    if (file != NULL)
    (
        /* Extract basename from file */
        for(p = basename = file; *p != '\0'; p++) {
            if(*p == '/' || *p == '\\') {
                basename = p + 1;
            }
        }
        
        INFO(basename);
    )
    sprintf(tmp, " %04d: |%d| ", line, level);
    INFO(tmp);
    INFO("\r\n");
}

static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
    return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Class
/**
 * HttpsRequest Constructor
 * Initializes the TCP socket, sets up event handlers and flags.
 *
 * @param[in] net_iface The network interface
 * @param[in] ssl_ca_pem String containing the trusted CAs
 * @param[in] method HTTP method to use
 * @param[in] url URL to the resource
 * @param[in] body_callback Callback on which to retrieve chunks of the response body.
                            If not set, the complete body will be allocated on the HttpResponse object,
                            which might use lots of memory.
 */
HttpsRequest::HttpsRequest(NetworkInterface* net_iface,
                           const char* ssl_ca_pem,
                           http_method method,
                           const char* url,
                           Callback<void(const char *at, size_t length)> body_callback)
    : _parsed_url(url),
      _tcpsocket(net_iface),
      _headerBuilder(method, &_parsed_url)
{
    _response = NULL;
    _ssl_ca_pem = ssl_ca_pem;
    _error = NSAPI_ERROR_OK;
    
    mbedtls_entropy_init(&_entropy);
    mbedtls_ctr_drbg_init(&_ctr_drbg);
    mbedtls_x509_crt_init(&_cacert);
    mbedtls_ssl_init(&_ssl);
    mbedtls_ssl_config_init(&_ssl_conf);
}

    /**
     * HttpsRequest Destructor
     */
HttpsRequest::~HttpsRequest()
{
    mbedtls_entropy_free(&_entropy);
    mbedtls_ctr_drbg_free(&_ctr_drbg);
    mbedtls_x509_crt_free(&_cacert);
    mbedtls_ssl_free(&_ssl);
    mbedtls_ssl_config_free(&_ssl_conf);
    
    _tcpsocket.close();
    
    if (_response)
    {
        delete _response;
    }
}

/**
 * Execute the HTTPS request.
 *
 * @param[in] body Pointer to the request body
 * @param[in] body_size Size of the request body
 * @return An HttpResponse pointer on success, or NULL on failure.
 *         See get_error() for the error code.
 */
HttpResponse* HttpsRequest::send(const void* body, size_t body_size) 
{
    if (body == NULL)
    {
        body_size = 0;
    }
    /*
     * Initialize TLS-related stuf.
     */
    int ret;
    if ((ret = mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy,
                      (const unsigned char *) TLS_CUNSTOM,
                      sizeof (TLS_CUNSTOM))) != 0)
    {
        ERROR("mbedtls_crt_drbg_init");
        _error = ret;
        return NULL;
    }

    if ((ret = mbedtls_x509_crt_parse(&_cacert, (const unsigned char *)_ssl_ca_pem,
                       strlen(_ssl_ca_pem) + 1)) != 0)
    {
        ERROR("mbedtls_x509_crt_parse");
        _error = ret;
        return NULL;
    }

    if ((ret = mbedtls_ssl_config_defaults(&_ssl_conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ERROR("mbedtls_ssl_config_defaults");
        _error = ret;
        return NULL;
    }

    mbedtls_ssl_conf_ca_chain(&_ssl_conf, &_cacert, NULL);
    mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, &_ctr_drbg);

    /* It is possible to disable authentication by passing
     * MBEDTLS_SSL_VERIFY_NONE in the call to mbedtls_ssl_conf_authmode()
     */
    mbedtls_ssl_conf_authmode(&_ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);

#if DEBUG_LEVEL > 0
    mbedtls_ssl_conf_verify(&_ssl_conf, my_verify, NULL);
    mbedtls_ssl_conf_dbg(&_ssl_conf, my_debug, NULL);
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

    if ((ret = mbedtls_ssl_setup(&_ssl, &_ssl_conf)) != 0)
    {
        ERROR("mbedtls_ssl_setup");
        _error = ret;
        return NULL;
    }
    
    mbedtls_ssl_set_hostname(&_ssl, _parsed_url.host());
    
    mbedtls_ssl_set_bio(&_ssl, static_cast<void *>(&_tcpsocket), ssl_send, ssl_recv, NULL );
    
    /* Connect to the server */
    ret = _tcpsocket.connect(_parsed_url.host(), _parsed_url.port());
    if (ret != NSAPI_ERROR_OK)
    {
        ERROR("Failed to connect");
        _error = -1;
        return NULL;
    }

   /* Start the handshake */
    ret = mbedtls_ssl_handshake(&_ssl);
    if (ret < 0) 
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) 
        {
            ERROR("mbedtls_ssl_handshake");
            ret = -1;
        }
        
        on_error(ret);
        return NULL;
    }
    
    /* Send the HTTP header */
    size_t request_size = 0;
    char* request = _headerBuilder.build(body_size, request_size);   
    ret = mbedtls_ssl_write(&_ssl, (const unsigned char *)request, request_size);
    if (check_mbedtls_ssl_write(ret)) 
    {
        return NULL;
    }
    free(request);

    /* Send body */
    char *send_buf = (char *)body;
    while (body_size > 0) 
    {
        size_t send_size = body_size < 4000 ? body_size : 4000; 
        ret = mbedtls_ssl_write(&_ssl, (const unsigned char *)send_buf, send_size);
        body_size -= send_size;
        send_buf += send_size;

        if (check_mbedtls_ssl_write(ret)) 
        {
            return NULL;
        }
    }
    mbedtls_ssl_write(&_ssl, (const unsigned char *)"\r\n", 2);
    if (check_mbedtls_ssl_write(ret)) 
    {
        return NULL;
    }
    
    char buf[1024];
    mbedtls_x509_crt_info(buf, sizeof(buf), "\r    ", mbedtls_ssl_get_peer_cert(&_ssl));
    buf[sizeof(buf) - 1] = 0;
    INFO("Server certificate:");
    INFO(buf);

    uint32_t flags = mbedtls_ssl_get_verify_result(&_ssl);
    if( flags != 0 )
    {
        mbedtls_x509_crt_verify_info(buf, 1024, "\r  ! ", flags);
        ERROR("Certificate verification failed");
    }
    else 
    {
        INFO("Certificate verification passed");
    }

    // Create a response object
    if (_response)
    {
        delete _response;
    }
    _response = new HttpResponse();
    // And a response parser
    HttpResponseParser parser(_response, _body_callback);

    // Set up a receive buffer (on the heap)
    uint8_t* recv_buffer = (uint8_t*)malloc(HTTP_RECEIVE_BUFFER_SIZE);

    /* Read data out of the socket */
    while ((ret = mbedtls_ssl_read(&_ssl, (unsigned char *) recv_buffer, HTTP_RECEIVE_BUFFER_SIZE)) > 0) 
    {
        // Don't know if this is actually needed, but OK
        size_t _bpos = static_cast<size_t>(ret);
        recv_buffer[_bpos] = 0;

        size_t nparsed = parser.execute((const char*)recv_buffer, _bpos);
        if (nparsed != _bpos) 
        {
            ERROR("parser_error");
            on_error(-2101);
            free(recv_buffer);
            return NULL;
        }
        // No more chunks? break out of this loop
        if (_bpos < HTTP_RECEIVE_BUFFER_SIZE) 
        {
            break;
        }
    }
    
    if (ret < 0) 
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            ERROR("mbedtls_ssl_read");
            ret = -1;
        }
        on_error(ret);
        free(recv_buffer);
        return NULL;
    }

    parser.finish();

    _tcpsocket.close();
    free(recv_buffer);

    return _response;
}

bool HttpsRequest::check_mbedtls_ssl_write(int ret)
{
    if (ret < 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            ERROR("mbedtls_ssl_write");
            ret = -1;
        }
        
        on_error(ret);
        return 1;
    }
    return 0;
}

/**
 * Set a header for the request.
 *
 * The 'Host' and 'Content-Length' headers are set automatically.
 * Setting the same header twice will overwrite the previous entry.
 *
 * @param[in] key Header key
 * @param[in] value Header value
 */
void HttpsRequest::set_header(const char* key, const char* value)
{
    _headerBuilder.set_header(key, value);
}

/**
 * Get the error code.
 *
 * When send() fails, this error is set.
 */
nsapi_error_t HttpsRequest::get_error()
{
    return _error;
}

void HttpsRequest::on_error(int error) 
{
    _tcpsocket.close();
    _error = error;
}
