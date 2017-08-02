// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __TELEMERTY_H__
#define __TELEMERTY_H__

#include "mbed.h"
#include "CircularBuffer.h"

#define MESSAGE_QUEUE_SIZE  5

/** Client to collect device telemetry data and send to Azure Application Insights 
*
*/
class TelemetryClient
{
public:
    /**
    Instantiate the telemetry client.
    @param ai_endoint the endpoint of the Azure Application Insights.
    @param ai_ikey the IKEY to connect with Azure Application Insights.
    */
    TelemetryClient(const char *ai_endoint, const char *ai_ikey);

    /**
    Send the telemetry data to Azure Application Insights.
    @param event the event name of the telemetry data.
    @param message the extra information of the telemetry data.
    @param iothub this is the iot hub name if the device connects with Azure IoT Hub, the name will be hashed before send to Azure Application Insights.
    */
    void Send(const char *event, const char *message = NULL, const char *iothub = NULL, bool async = true);

private:
    void telemetry_worker(void);

    void hash(char *result, const char *input);
    void send_data_to_ai(const char* data, int size);
    void do_trace_telemetry(const char *iothub, const char *event, const char *message, bool async);

    bool push_msg(char* message);
    char* pop_msg(void);

private:
    const char* m_ai_endoint;
    const char* m_ai_ikey;

    char m_hash_mac[36];
    char m_hash_iothub_name[36];
    int m_base_size;

    CircularBuffer<char*, MESSAGE_QUEUE_SIZE> m_pool;
        
    Thread m_telemetry_thread;
};


#endif  // __TELEMERTY_H__