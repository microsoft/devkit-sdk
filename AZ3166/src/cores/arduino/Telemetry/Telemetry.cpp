// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "Arduino.h"
#include "Telemetry.h"
#include "TelemetryClient.h"

#ifndef ENABLETRACE
// By default trace is enabled
#define ENABLETRACE           1
#endif

// Todo, the url and key of AI shall be get from REST service / web page, instead of hardcode here 
static const char *AI_ENDPOINT = "https://dc.services.visualstudio.com/v2/track";
static const char *AI_IKEY = "63d78aab-86a7-49b9-855f-3bdcff5d39d7";

static TelemetryClient *telemetry = NULL;

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

void telemetry_init()
{
    if (telemetry == NULL && ENABLETRACE)
    {
        telemetry = new TelemetryClient(AI_ENDPOINT, AI_IKEY);
    }
}

void send_telemetry_data(const char *iothub, const char *event, const char *message)
{
    if (telemetry && ENABLETRACE)
    {
        telemetry->Send(event, message, iothub, true);
    }
}

void send_telemetry_data_async(const char *iothub, const char *event, const char *message)
{
    if (telemetry && ENABLETRACE)
    {
        telemetry->Send(event, message, iothub, true);
    }
}

void send_telemetry_data_sync(const char *iothub, const char *event, const char *message)
{
    if (telemetry && ENABLETRACE)
    {
        telemetry->Send(event, message, iothub, false);
    }
}

#ifdef __cplusplus
}
#endif  // __cplusplus