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

#ifndef _HTTP_HEADER_BUILDER_H_
#define _HTTP_HEADER_BUILDER_H_

#include "http_common.h"
#include "http_parsed_url.h"

class HttpHeaderBuilder 
{
public:
    HttpHeaderBuilder(http_method method, ParsedUrl* parsed_url);
    virtual ~HttpHeaderBuilder();
    
    void set_header(const char* key, const char* value);

    char* build(size_t body_size, size_t &size);

    void free_headers(char* data);

private:
    http_method _method;
    ParsedUrl* _parsed_url;
    
    KEYVALUE *headers;
};

#endif // _HTTP_HEADER_BUILDER_H_
