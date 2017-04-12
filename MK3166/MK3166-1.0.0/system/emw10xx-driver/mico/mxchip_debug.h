/* MiCO Team
 * Copyright (c) 2017 MXCHIP Information Tech. Co.,Ltd
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

#ifndef __MXCHIP_DEBUG_H__
#define __MXCHIP_DEBUG_H__

typedef int (*debug_printf)( char*msg, ... );

enum {
    SYSTEM_DEBUG_ERROR = 1,
    SYSTEM_DEBUG_DEBUG = 2,
    SYSTEM_DEBUG_INFO  = 3,
};

extern debug_printf pPrintffunc;
extern int debug_level;

#define system_debug_printf(level, ...) \
do {\
    if ((level <= debug_level) && (pPrintffunc != NULL))\
        pPrintffunc(__VA_ARGS__);\
}while(0)


#define cmd_printf(...) do{\
                                if (xWriteBufferLen > 0) {\
                                    snprintf(pcWriteBuffer, xWriteBufferLen, __VA_ARGS__);\
                                    xWriteBufferLen-=strlen(pcWriteBuffer);\
                                    pcWriteBuffer+=strlen(pcWriteBuffer);\
                                }\
                             }while(0)


void system_debug_enable(int level, debug_printf callback);



#endif
