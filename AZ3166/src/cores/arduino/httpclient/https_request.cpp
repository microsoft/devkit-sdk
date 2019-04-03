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
{
    _parsed_url = NULL;
    _body_callback = body_callback;
    _tlssocket = NULL;
    _headerBuilder = NULL;
    _response = NULL;
    _error = NSAPI_ERROR_OK;

    _parsed_url = new ParsedUrl(url);
    _tlssocket = new TLSSocket(ssl_ca_pem, net_iface);
    _headerBuilder = new HttpHeaderBuilder(method, _parsed_url);
}

    /**
     * HttpsRequest Destructor
     */
HttpsRequest::~HttpsRequest()
{
    if (_response)
    {
        delete _response;
    }
    
    if (_parsed_url)
    {
        delete _parsed_url;
    }
        
    if (_tlssocket)
    {
        delete _tlssocket;
    }
        
    if (_headerBuilder)
    {
        delete _headerBuilder;
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
	
	// Connect to the HTTP(S) server
    _error = _tlssocket->connect(_parsed_url->host(), _parsed_url->port());
    if (_error != NSAPI_ERROR_OK)
    {
        ERROR("Failed to connect");
        return NULL;
    }
    
    /* Send the HTTP header */
    size_t request_size = 0;
    char* request = _headerBuilder->build(body_size, request_size);
    _error = _tlssocket->send(request, request_size);
    if ((size_t)_error != request_size)
    {
        ERROR("Failed to send the HTTP header");
        _tlssocket->close();
        return NULL;
    }
    _headerBuilder->free_headers(request);
    
    /* Send body */
    char *send_buf = (char *)body;
    while (body_size > 0) 
    {
        size_t send_size = body_size < 4000 ? body_size : 4000; 
        _error = _tlssocket->send(send_buf, send_size);
        if (_error < 0)
        {
            ERROR("Failed to send the HTTP body");
            _tlssocket->close();
            return NULL;
        }
        
        body_size -= send_size;
        send_buf += send_size;
    }
    _error = _tlssocket->send((const unsigned char *)"\r\n", 2);
    if (_error < 0)
    {
        ERROR("Failed to send the ending");
        _tlssocket->close();
        return NULL;
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
    uint8_t* recv_buffer = new uint8_t[HTTP_RECEIVE_BUFFER_SIZE + 1];

    /* Read data out of the socket */
    int recved = 0;
    while ((recved = _tlssocket->recv((unsigned char *)recv_buffer, HTTP_RECEIVE_BUFFER_SIZE)) > 0) 
    {
        // Don't know if this is actually needed, but OK
        size_t _bpos = static_cast<size_t>(recved);
        recv_buffer[_bpos] = 0;
        
        size_t nparsed = parser.execute((const char*)recv_buffer, _bpos);
        if (nparsed != _bpos) 
        {
            ERROR("parser_error");
            _error = -2101;
            _tlssocket->close();
            delete [] recv_buffer;
            return NULL;
        }
        // break out of this loop
        if (_response->is_message_complete()) {
            break;
        }
    }
    parser.finish();
    _tlssocket->close();
    delete [] recv_buffer;
    
    if (recved < 0) 
    {
        return NULL;
    }
    else
    {
        return _response;
    }
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
    _headerBuilder->set_header(key, value);
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

