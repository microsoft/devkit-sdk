// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "../inc/azure_c_shared_utility/xlogging.h"

#include "SerialLog.h"

void consolelogger_log(LOG_CATEGORY log_category, const char* file, const char* func, const int line, unsigned int options, const char* format, ...)
{
    va_list arg;
    char temp[64];
    char* buffer = temp;
    
    va_start(arg, format);
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    
    if (len > sizeof(temp) - 1)
    {
        buffer = (char*)malloc(len + 1);
        if (!buffer)
        {
            return;
        }

        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    
    switch (log_category)
    {
    case AZ_LOG_INFO:
        (void)serial_log("Info: ");
        break;
    case AZ_LOG_ERROR:
        {
            time_t t = time(NULL); 
            (void)serial_xlog("Error: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), file, func, line);
        }
        break;
    default:
        break;
    }

    serial_log(buffer);
    
    if (options & LOG_LINE)
	{
		(void)serial_log("\r\n");
	}

    if (buffer != temp)
    {
        free(buffer);
    }	
}
