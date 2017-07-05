// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTME_WIFI_H__
#define __SYSTME_WIFI_H__

#include "mbed.h"

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

bool InitSystemWiFi(void);
bool SystemWiFiConnect(void);
const char* SystemWiFiSSID(void);
NetworkInterface* WiFiInterface(void);

bool SystemWiFiAPStart(const char *ssid, const char *passphrase);
NetworkInterface* WiFiAPInterface(void);

int WiFiScan(WiFiAccessPoint *res, unsigned count);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __SYSTME_WIFI_H__
