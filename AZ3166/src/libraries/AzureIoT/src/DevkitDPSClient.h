// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#ifndef __DEVKIT_DPS_CLIENT_H__
#define __DEVKIT_DPS_CLIENT_H__

#include "AzureIotHub.h"

#ifdef __cplusplus
extern "C"
{
#endif

bool DevkitDPSClientStart(const char* dps_uri, const char* dps_scope_id, const char* RegistrationId = NULL, char* udsString = NULL, const char* proxy_address = NULL, int proxy_port = 0);
char* DevkitDPSGetIoTHubURI(void);
char* DevkitDPSGetDeviceID(void);

#ifdef __cplusplus
}
#endif

#endif // __DPS_CLIENT_H__
