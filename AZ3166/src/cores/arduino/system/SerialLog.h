// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SERIAL_LOG_H__
#define __SERIAL_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

void serial_log(const char* msg);

void serial_xlog(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif  // __SERIAL_LOG_H__
