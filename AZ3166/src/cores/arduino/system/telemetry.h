// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef _TELEMETRY_H_
#define _TELEMETRY_H_

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus


    void telemetry_init();
    void send_telemetry_data(const char *iothub, const char *event, const char *message);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // _TELEMETRY_H_