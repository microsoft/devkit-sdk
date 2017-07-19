// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

void serial_log(const char* msg)
{
    if (msg != NULL)
    {
        Serial.print(msg);
    }
}

void serial_xlog(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1)
    {
        buffer = new char[len + 1];
        if (!buffer)
        {
            return;
        }

        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }

    Serial.print(buffer);
    
    if (buffer != temp)
    {
        delete[] buffer;
    }
}

#ifdef __cplusplus
}
#endif