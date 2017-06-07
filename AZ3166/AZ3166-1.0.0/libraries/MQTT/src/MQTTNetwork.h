#ifndef _MQTTNETWORK_H_
#define _MQTTNETWORK_H_

#include "NetworkInterface.h"
#include "MQTTmbed.h"
#include "SystemWiFi.h"
#include "telemetry.h"

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
        //send_telemetry_data("", "mqtt connection", telemetry);
        return ret;
    }

    void disconnect() {

    }

private:
    TCPSocket* socket;
};

#endif // _MQTTNETWORK_H_
