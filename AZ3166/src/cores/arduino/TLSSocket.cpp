// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "TLSSocket.h"

#define TLS_CUNSTOM "Arduino TLS Socket"

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
static void my_debug(void *ctx, int level, const char *file_name, int line, const char *str)
{
    char tmp[32];
    const char *p, *basename;
    
    if (file_name != NULL)
    {
        /* Extract basename from file */
        basename = file_name;
        for (p = basename; *p != '\0'; p++)
        {
            if(*p == '/' || *p == '\\')
            {
                basename = p + 1;
            }
        }
        
        INFO(basename);
    }
    sprintf(tmp, " %04d: |%d| ", line, level);
    INFO(tmp);
    INFO("\r\n");
}

static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
    const uint32_t buf_size = 1024;
    char *buf = new char[buf_size];
    (void) data;

    
    mbedtls_x509_crt_info(buf, buf_size - 1, "  ", crt);
    

    if (*flags == 0)
    {
        INFO("No verification issue for this certificate");
    }
    else
    {
        mbedtls_x509_crt_verify_info(buf, buf_size, "  ! ", *flags);
        INFO(buf);
    }

    delete[] buf;
    return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Class
TLSSocket::TLSSocket(const char *ssl_ca_pem, NetworkInterface* net_iface)
{
    _ssl_ca_pem = ssl_ca_pem;
    
    if (net_iface)
    {
        _tcp_socket = new TCPSocket(net_iface);
    }
    else
    {
        _tcp_socket = NULL;
    }

    if (ssl_ca_pem)
    {
        // SSL
        mbedtls_entropy_init(&_entropy);
        mbedtls_ctr_drbg_init(&_ctr_drbg);
        mbedtls_x509_crt_init(&_cacert);
        mbedtls_ssl_init(&_ssl);
        mbedtls_ssl_config_init(&_ssl_conf);
    }
}

TLSSocket::~TLSSocket()
{
    if (_ssl_ca_pem)
    {
        mbedtls_entropy_free(&_entropy);
        mbedtls_ctr_drbg_free(&_ctr_drbg);
        mbedtls_x509_crt_free(&_cacert);
        mbedtls_ssl_free(&_ssl);
        mbedtls_ssl_config_free(&_ssl_conf);
    }
    
    if (_tcp_socket)
    {
        _tcp_socket->close();
        delete _tcp_socket;
    }
}

nsapi_error_t TLSSocket::connect(const char *host, uint16_t port)
{
    if (_tcp_socket == NULL)
    {
        return NSAPI_ERROR_NO_SOCKET;
    }
    
    if (_ssl_ca_pem == NULL)
    {
        // No SSL
        return _tcp_socket->connect(host, port);
    }
    
    // Initialize TLS-related stuf.
    int ret;
    if ((ret = mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy,
                      (const unsigned char *) TLS_CUNSTOM,
                      sizeof (TLS_CUNSTOM))) != 0)
    {
        return -1;
    }

    if ((ret = mbedtls_x509_crt_parse(&_cacert, (const unsigned char *)_ssl_ca_pem,
                       strlen(_ssl_ca_pem) + 1)) != 0)
    {
        return -1;
    }

    if ((ret = mbedtls_ssl_config_defaults(&_ssl_conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        return -1;
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
        return -1;
    }
    
    mbedtls_ssl_set_hostname(&_ssl, host);
    
    mbedtls_ssl_set_bio(&_ssl, static_cast<void *>(_tcp_socket), ssl_send, ssl_recv, NULL );
    
    /* Connect to the server */
    ret = _tcp_socket->connect(host, port);
    if (ret != NSAPI_ERROR_OK)
    {
        return ret;
    }

   /* Start the handshake */
    ret = mbedtls_ssl_handshake(&_ssl);
    if (ret < 0) 
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
            ret != MBEDTLS_ERR_SSL_WANT_WRITE) 
        {
            ret = -1;
        }
        
        return ret;
    }
    
    return NSAPI_ERROR_OK;
}

nsapi_error_t TLSSocket::close()
{
    if (_tcp_socket == NULL)
    {
        return NSAPI_ERROR_NO_SOCKET;
    }
    return _tcp_socket->close();
}

nsapi_size_or_error_t TLSSocket::send(const void *data, nsapi_size_t size)
{
    if (_tcp_socket == NULL)
    {
        return NSAPI_ERROR_NO_SOCKET;
    }
    
    if (_ssl_ca_pem == NULL)
    {
        // No SSL
        const unsigned char *ptr = (const unsigned char *)data;
        int result, data_size = size;
        while((result = ssl_send(_tcp_socket, ptr, data_size)) > 0)
        {
            ptr += result;
            data_size -= result;
            if (data_size == 0) return size;
        }
        return result;
    }

    return mbedtls_ssl_write(&_ssl, (const unsigned char*)data, size);
}

nsapi_size_or_error_t TLSSocket::recv(void *data, nsapi_size_t size)
{
    if (_tcp_socket == NULL)
    {
        return NSAPI_ERROR_NO_SOCKET;
    }
    
    if (_ssl_ca_pem == NULL)
    {
        // No SSL
        return _tcp_socket->recv(data, size);
    }

    return mbedtls_ssl_read(&_ssl, (unsigned char*)data, size);
}
