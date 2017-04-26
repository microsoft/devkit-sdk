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

#include "http_response.h"

HttpResponse::HttpResponse()
{
    status_code = 0;
    status_message = NULL;
    body = NULL;
    headers = NULL;
    concat_header_field = false;
    concat_header_value = false;
}

HttpResponse::~HttpResponse()
{
    if (status_message)
    {
        free(status_message);
    }
    if (body)
    {
        free(body);
    }
    while (headers != NULL)
    {
        KEYVALUE* prev = headers->prev;
        free(headers->key);
        free(headers->value);
        free(headers);
        headers = prev;
    }
}

void HttpResponse::set_status(int a_status_code, const char *status_message_at, size_t status_message_length) 
{
    status_code = a_status_code;
    if (status_message_at != NULL && status_message_length > 0)
    {
        if (status_message != NULL)
        {
            free(status_message);
        }
        status_message = malloc(status_message_length + 1);
        memcpy(status_message, status_message_at, status_message_length);
        status_message[status_message_length] = 0;
    }
}

int HttpResponse::get_status_code()
{
    return status_code;
}

string HttpResponse::get_status_message()
{
    return status_message;
}

void HttpResponse::set_header_field(const char* field_at, size_t length) 
{
    if (field_at == NULL || length == 0)
    {
        return;
    }
    
    concat_header_value = false;
    
    // headers can be chunked
    char* key = malloc(length + 1);
    memcpy(key, field_at, length);
    key[length] = 0;
    if (concat_header_field) 
    {
        if (headers->key)
        {
            free(headers->key);
        }
        headers->key = key;
    }
    else
    {
        // New
        KEYVALUE* header = (KEYVALUE*)malloc(sizeof(KEYVALUE));
        header->key = key;
        header->value = NULL;
        header->prev = headers;
        headers = header;
    }

    concat_header_field = true;
}

void HttpResponse::set_header_value(const char* value_at, size_t length) 
{
    if (value_at == NULL || length == NULL)
    {
        return;
    }
    
    concat_header_field = false;

    // headers can be chunked
    char* value = malloc(length + 1);
    memcpy(value, value_at, length);
    value[length] = 0;
    if (concat_header_value) 
    {
        if (headers->value)
        {
            free(headers->value);
        }
        headers->value = value;
    }
    else 
    {
        // New
        KEYVALUE* header = (KEYVALUE*)malloc(sizeof(KEYVALUE));
        header->key = NULL;
        header->value = value;
        header->prev = headers;
        headers = header;
    }
    
    concat_header_value = true;
}

const KEYVALUE* HttpResponse::get_headers()
{
    return headers;
}

void HttpResponse::set_body(const char* at, size_t length) 
{
    if (at == NULL || length == 0)
    {
        return;
    }
    
    if(body != NULL)
    {
        int len1 = strlen(body);
        char* bd = malloc(len1 + length + 1);
        memcpy(bd, body, len1);
        memcpy(&bd[len1], at, length);
        bd[len1 + length] = 0;
        free(body);
        body = bd;
    }
    else
    {
        body = malloc(length + 1);
        memcpy(body, at, length);
        body[length] = 0;
    }
}

string HttpResponse::get_body()
{
    return body;
}
