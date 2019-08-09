// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_WEB_H__
#define __SYSTEM_WEB_H__

#include "mbed.h"

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#define WEB_SETTING_IOT_DEVICE_CONN_STRING  0x00000001
#define WEB_SETTING_IOT_CERT                0x00000002
#define WEB_SETTING_IOT_DPS_SYMMETRIC_KEY   0x00000004

void EnableSystemWeb(int extFunctions);

void StartupSystemWeb(void);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTEM_WEB_H__
