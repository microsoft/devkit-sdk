/* 
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

 
#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include "http_common.h"
#include "https_request.h"

typedef struct
{
    int status_code;
    int body_length;
    const char* status_message;
    const char* body;
    const KEYVALUE* headers;
} Http_Response;

class HTTPClient 
{
public:
    HTTPClient(http_method method, const char* url, Callback<void(const char *at, size_t length)> body_callback = 0);
    HTTPClient(const char* ssl_ca_pem, http_method method, const char* url, Callback<void(const char *at, size_t length)> body_callback = 0);
    virtual ~HTTPClient(void);
    
    const Http_Response* send(const void* body = NULL, int body_size = 0);
    void set_header(const char* key, const char* value);
    nsapi_error_t get_error();
    
private:
    void init(const char* ssl_ca_pem, http_method method, const char* url, Callback<void(const char *at, size_t length)> body_callback);
    
    HttpsRequest *_https_request;
    Http_Response *_response;
};

#endif