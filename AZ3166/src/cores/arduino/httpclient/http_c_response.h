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

#ifndef __HTTP_C_RESPONSE_2017_4_29__
#define __HTTP_C_RESPONSE_2017_4_29__

#include "http_common.h"

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void set_status(int a_status_code, const char *status_message_at, size_t status_message_length);

    int get_status_code();

    const char* get_status_message();

    void set_header_field(const char* field_at, size_t length);

    void set_header_value(const char* value_at, size_t length);
    
    const KEYVALUE* get_headers();
    
    void set_body(const char* at, size_t length);

    const char* get_body();

    int get_body_length();

    void set_message_complete();

    bool is_message_complete();

private:
    int status_code;
    char* status_message;
    
    KEYVALUE *headers;
        
    bool concat_header_field;
    bool concat_header_value;
    bool is_message_completed;

    char* body;
    int body_length = 0;
};
#endif  // __HTTP_C_RESPONSE_2017_4_29__
