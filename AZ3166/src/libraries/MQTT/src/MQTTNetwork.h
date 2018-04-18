// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef _MQTTNETWORK_H_
#define _MQTTNETWORK_H_

#include "NetworkInterface.h"
#include "MQTTmbed.h"
#include "SystemWiFi.h"
#include "Telemetry.h"

class MQTTNetwork
{
  public:
    MQTTNetwork()
    {
        _tcpSocket = NULL;
    }

    ~MQTTNetwork()
    {
        if (_tcpSocket != NULL)
        {
            _tcpSocket->close();
            delete _tcpSocket;
            _tcpSocket = NULL;
        }
    }

    int read(unsigned char *buffer, int len, int timeout)
    {
        return _tcpSocket->recv(buffer, len);
    }

    int write(unsigned char *buffer, int len, int timeout)
    {
        return _tcpSocket->send(buffer, len);
    }

    int connect(const char *hostname, int port)
    {
        if (_tcpSocket != NULL)
        {
            return NSAPI_ERROR_OK;
        }
        else
        {
            _tcpSocket = new TCPSocket();
            if (_tcpSocket == NULL)
            {
                return NSAPI_ERROR_NO_SOCKET;
            }

            int ret;

            if ((ret = _tcpSocket->open(WiFiInterface())) == 0 && (ret = _tcpSocket->connect(hostname, port)) == 0)
            {
                char telemetry[128];
                snprintf(telemetry, 128, "ret: %d, host: %s", ret, hostname);

                // Microsoft collects data to operate effectively and provide you the best experiences with our products.
                // We collect data about the features you use, how often you use them, and how you use them.
                send_telemetry_data_async("", "mqtt connection", telemetry);
            }
            else
            {
                // open socket failed or connect host failed
                delete _tcpSocket;
                _tcpSocket = NULL;
            }

            return ret;
        }
    }

    int disconnect()
    {
        if (_tcpSocket == NULL)
        {
            return NSAPI_ERROR_OK;
        }
        else
        {
            int ret;
            if ((ret = _tcpSocket->close()) != NSAPI_ERROR_OK)
            {
                return ret;
            }

            delete _tcpSocket;
            _tcpSocket = NULL;
            return NSAPI_ERROR_OK;
        }
    }

  private:
    TCPSocket *_tcpSocket;
};

#endif // _MQTTNETWORK_H_
