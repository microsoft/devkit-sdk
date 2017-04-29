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

#ifndef __HTTPS_REQUEST_H__
#define __HTTPS_REQUEST_H__

#include "http_common.h"
#include "http_c_response.h"
#include "http_header_builder.h"
#include "http_parsed_url.h"

#include "TLSSocket.h"

/**
 * \brief HttpsRequest implements the logic for interacting with HTTP(S) servers.
 */
class HttpsRequest 
{
public:
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
    HttpsRequest(NetworkInterface* net_iface,
                 const char* ssl_ca_pem,
                 http_method method,
                 const char* url,
                 Callback<void(const char *at, size_t length)> body_callback = 0);
    
    /**
     * HttpsRequest Destructor
     */
    ~HttpsRequest();

    /**
     * Execute the HTTPS request.
     *
     * @param[in] body Pointer to the request body
     * @param[in] body_size Size of the request body
     * @return An HttpResponse pointer on success, or NULL on failure.
     *         See get_error() for the error code.
     */
    HttpResponse* send(const void* body = NULL, nsapi_size_t body_size = 0);
    
    /**
     * Set a header for the request.
     *
     * The 'Host' and 'Content-Length' headers are set automatically.
     * Setting the same header twice will overwrite the previous entry.
     *
     * @param[in] key Header key
     * @param[in] value Header value
     */
    void set_header(const char* key, const char* value);

    /**
     * Get the error code.
     *
     * When send() fails, this error is set.
     */
    nsapi_error_t get_error();
    
private:
    ParsedUrl *_parsed_url;
    TLSSocket *_tlssocket;
    HttpHeaderBuilder *_headerBuilder;
    
    Callback<void(const char *at, size_t length)> _body_callback;
    HttpResponse* _response;
    
    nsapi_error_t _error;
};

#endif // __HTTPS_REQUEST_H__
