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

#ifndef _MBED_HTTP_PARSED_URL_H_
#define _MBED_HTTP_PARSED_URL_H_

#include "http_common.h"

class ParsedUrl 
{
public:
    ParsedUrl(const char* url);

    ~ParsedUrl();
    
    uint16_t port() const { return _port; }
    const char* schema() const { return _schema; }
    const char* host() const { return _host; }
    const char* path() const { return _path; }
    const char* query() const { return _query; }
    const char* userinfo() const { return _userinfo; }

private:
    uint16_t _port;
    const char* _schema;
    const char* _host;
    const char* _path;
    const char* _query;
    const char* _userinfo;
};

#endif // _MBED_HTTP_PARSED_URL_H_
