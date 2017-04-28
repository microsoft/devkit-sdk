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

#ifndef __HTTPS_COMMON_H__
#define __HTTPS_COMMON_H__

#define DEBUG_LEVEL 0

typedef struct _tagKeyValue
{
    char* key;
    char* value;
    
    struct _tagKeyValue* prev;
} KEYVALUE;

#include "mbed.h"
#include "http_parser.h"

#if DEBUG_LEVEL > 0
#define ERROR(x) do { printf("ERROR: "); printf(x); printf("\r\n"); } while(0);
#else
#define ERROR(x) do {  } while(0);
#endif

#if DEBUG_LEVEL > 1
#define INFO(x) do { printf(x); printf("\r\n"); } while(0);
#else
#define INFO(x) do {  } while(0);
#endif

#define HTTP_RECEIVE_BUFFER_SIZE 2048


#endif // __HTTPS_COMMON_H__
