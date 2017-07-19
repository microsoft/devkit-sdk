// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef TELEMERTY_H
#define TELEMERTY_H

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus


    void telemetry_init();
    void send_telemetry_data(const char *iothub, const char *event, const char *message);

#ifdef __cplusplus
}
#endif

#endif