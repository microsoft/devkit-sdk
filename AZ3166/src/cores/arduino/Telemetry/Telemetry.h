// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_TELEMERTY_H__
#define __SYSTEM_TELEMERTY_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

    // Initialize the system telemetry
    void telemetry_init();
    
    // Send an telemetry data to AI asynchronously
    void send_telemetry_data(const char *iothub, const char *event, const char *message);
        
    // Send an telemetry data to AI asynchronously
    void send_telemetry_data_async(const char *iothub, const char *event, const char *message);
    
    // Send an telemetry data to AI synchronously
    void send_telemetry_data_sync(const char *iothub, const char *event, const char *message);

#ifdef __cplusplus
}
#endif

#endif // __SYSTEM_TELEMERTY_H__