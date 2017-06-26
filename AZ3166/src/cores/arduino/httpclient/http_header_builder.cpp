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
#include "http_header_builder.h"

#define KEV_PAIR_LEN(x) strlen(x->key) + 1 + 1 + strlen(x->value) + 2;      // KEY: VALUE\r\n

HttpHeaderBuilder::HttpHeaderBuilder(http_method method, ParsedUrl* parsed_url)
{
    _method = method;
    _parsed_url = parsed_url;
    headers = NULL;
    set_header("Host", _parsed_url->host());
}

HttpHeaderBuilder::~HttpHeaderBuilder()
{
    while (headers != NULL)
    {
        KEYVALUE* prev = headers->prev;
        free(headers->key);
        free(headers->value);
        free(headers);
        headers = prev;
    }
}

/**
 * Set a header for the request
 * If the key already exists, it will be overwritten...
 */
void HttpHeaderBuilder::set_header(const char* key, const char* value)
{
    if (key == NULL || value == NULL)
    {
        return;
    }
    
    KEYVALUE* header = headers;
    
    while (header != NULL)
    {
        if (strcmp(header->key, key) == 0)
        {
            // Same key, replace the value
            free(header->value);
            header->value = strdup(value);
            return;
        }
        header = header->prev;
    }
    
    // New
    header = (KEYVALUE*)malloc(sizeof(KEYVALUE));
    header->key = strdup(key);
    header->value = strdup(value);
    header->prev = headers;
    headers = header;
}

char* HttpHeaderBuilder::build(size_t body_size, size_t &size)
{
    const char* method_str = http_method_str(_method);

    if (_method == HTTP_POST || _method == HTTP_PUT) 
    {
        char buffer[10];
        snprintf(buffer, 10, "%d", body_size);
        set_header("Content-Length", buffer);
    }

    // first line is METHOD PATH+QUERY HTTP/1.1\r\n
    int len_first_line = strlen(method_str) + 1 + strlen(_parsed_url->path()) + (strlen(_parsed_url->query()) ? strlen(_parsed_url->query()) + 1 : 0) + 1 + 8 + 2;
    size += len_first_line;

    // after that we'll do the headers
    KEYVALUE* header = headers;
    while (header != NULL)
    {
        // line is KEY: VALUE\r\n
        size += KEV_PAIR_LEN(header);
        header = header->prev;
    }
    
    // then the body, first an extra newline
    size += 2;

    // Now let's print it
    char* req = (char*)calloc(size + 1, 1);
    if (req == NULL) 
    {
        ERROR("calloc failed");
    }
    char* originalReq = req;

    if (strlen(_parsed_url->query())) 
    {
        sprintf(req, "%s %s?%s HTTP/1.1\r\n", method_str, _parsed_url->path(), _parsed_url->query());
    } else 
    {
        sprintf(req, "%s %s%s HTTP/1.1\r\n", method_str, _parsed_url->path(), _parsed_url->query());
    }
    req += len_first_line;

    header = headers;
    while (header != NULL)
    {
        // line is KEY: VALUE\r\n
        sprintf(req, "%s: %s\r\n", header->key, header->value);
        req += KEV_PAIR_LEN(header);
        
        header = header->prev;
    }
    
    req[0] = '\r';
    req[1] = '\n';
    sprintf(req, "\r\n");
    req += 2;
    
    INFO(originalReq);
    
    return originalReq;
}

void HttpHeaderBuilder::free_headers(char* data)
{
    free(data);
}
