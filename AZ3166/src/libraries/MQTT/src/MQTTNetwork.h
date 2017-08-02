// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef _MQTTNETWORK_H_
#define _MQTTNETWORK_H_

#include "NetworkInterface.h"
#include "MQTTmbed.h"
#include "SystemWiFi.h"
#include "Telemetry.h"

class MQTTNetwork {
public:
    MQTTNetwork() {
        socket = new TCPSocket();
    }

    ~MQTTNetwork() {
        delete socket;
    }

    int read(unsigned char* buffer, int len, int timeout) {
        return socket->recv(buffer, len);
    }

    int write(unsigned char* buffer, int len, int timeout) {
        return socket->send(buffer, len);
    }

    int connect(const char* hostname, int port) {
        int ret;
        if ( (ret = socket->open(WiFiInterface())) != 0) {
            return ret;
        }
        ret = socket->connect(hostname, port);
        char telemetry[128];
        snprintf(telemetry, 128, "ret: %d, host: %s", ret, hostname);

        // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
        // We collect data about the features you use, how often you use them, and how you use them.
        send_telemetry_data_async("", "mqtt connection", telemetry);
        
        return ret;
    }

    void disconnect() {

    }

private:
    TCPSocket* socket;
};

#endif // _MQTTNETWORK_H_
