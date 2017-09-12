// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#ifndef __AZURE_IOTHUB_H__
#define __AZURE_IOTHUB_H__

#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xlogging.h"
#include "iothubtransportmqtt.h"
#include "azureiotcerts.h"

typedef void (*CONNECTION_STATUS_CALLBACK)(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason);
typedef void (*SEND_CONFIRMATION_CALLBACK)(IOTHUB_CLIENT_CONFIRMATION_RESULT result);
typedef void (*MESSAGE_CALLBACK)(const char* message, int length);
typedef void (*DEVICE_TWIN_CALLBACK)(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int length);
typedef int  (*DEVICE_METHOD_CALLBACK)(const char *methodName, const unsigned char *payload, int length, unsigned char **response, int *responseLength);
typedef void (*REPORT_CONFIRMATION_CALLBACK)(int status_code);
#endif // __AZURE_IOTHUB_H__