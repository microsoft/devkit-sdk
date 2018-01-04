// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#ifndef __DPS_CLIENT_H__
#define __DPS_CLIENT_H__

#include "AzureIotHub.h"

#ifdef __cplusplus
extern "C"
{
#endif

bool DPSClientStart(const char* dps_uri, const char* dps_scope_id, const char* proxy_address = NULL, int proxy_port = 0);
char* DPSGetIoTHubURI(void);
char* DPSGetDeviceID(void);

#ifdef __cplusplus
}
#endif

#endif // __DPS_CLIENT_H__
