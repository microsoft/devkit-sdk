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

#ifndef _HTTP_RESPONSE_PARSER_H_
#define _HTTP_RESPONSE_PARSER_H_

#include "http_common.h"
#include "http_c_response.h"

class HttpResponseParser 
{
public:
    HttpResponseParser(HttpResponse* a_response, Callback<void(const char *at, size_t length)> a_body_callback = 0);

    ~HttpResponseParser();

    size_t execute(const char* buffer, size_t buffer_size);

    void finish();

public:
    // Member functions
    int on_message_begin(http_parser* parser);
    int on_url(http_parser* parser, const char *at, size_t length);
    int on_status(http_parser* parser, const char *at, size_t length);
    int on_header_field(http_parser* parser, const char *at, size_t length);
    int on_header_value(http_parser* parser, const char *at, size_t length);
    int on_headers_complete(http_parser* parser);
    int on_body(http_parser* parser, const char *at, size_t length);
    int on_message_complete(http_parser* parser);
    int on_chunk_header(http_parser* parser);
    int on_chunk_complete(http_parser* parser);

private:
    Callback<void(const char *at, size_t length)> body_callback;
    http_parser *parser;
    http_parser_settings *settings;
    
    HttpResponse* response;
};

#endif // _HTTP_RESPONSE_PARSER_H_
