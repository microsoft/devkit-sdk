#ifndef _MQTTNETWORK_H_
#define _MQTTNETWORK_H_

#include "NetworkInterface.h"
#include "MQTTmbed.h"
#include "SystemWiFi.h"

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
        return socket->connect(hostname, port);
    }

    void disconnect() {

    }

private:
    TCPSocket* socket;
};

#endif // _MQTTNETWORK_H_
