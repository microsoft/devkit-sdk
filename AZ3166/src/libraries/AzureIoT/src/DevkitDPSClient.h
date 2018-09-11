// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#ifndef __DEVKIT_DPS_CLIENT_H__
#define __DEVKIT_DPS_CLIENT_H__

#include "AzureIotHub.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum DPS_AUTH_TYPE
{
    NO_DPS_AUTH = 0,
    DPS_AUTH_X509_INDIVIDUAL,
    DPS_AUTH_X509_GROUP,
    DPS_AUTH_SYMMETRIC_KEY
};

// this interface is only effective if you call before DevkitDPSClientStart
// by default it's on
void DevkitDPSSetLogTrace(bool ison);

// by default it's DPS_AUTH_X509_INDIVIDUAL
// you must set auth type before calling DevkitDPSClientStart
void DevkitDPSSetAuthType(DPS_AUTH_TYPE auth_type);

bool DevkitDPSClientStart(const char* dps_uri, const char* dps_scope_id, const char* registration_id = NULL,
    char* udsString = NULL, const char* proxy_address = NULL, int proxy_port = 0);

char* DevkitDPSGetIoTHubURI(void);
char* DevkitDPSGetDeviceID(void);

#ifdef __cplusplus
}
#endif

#endif // __DPS_CLIENT_H__
