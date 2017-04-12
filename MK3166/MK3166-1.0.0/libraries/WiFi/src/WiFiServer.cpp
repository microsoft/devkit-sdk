/*
  WiFiServer.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string.h>

extern "C" {
#include "utility/debug.h"
}

#include "AzureBoardWiFi.h"
#include "WiFiServer.h"
#include "utility\wifi_drv.h"
#include "IPAddress.h"

WiFiServer::WiFiServer(uint16_t port) : _port(port), _currentClient(nullptr)
{
    close();
    _pTcpServer = new TCPServer();
}

void WiFiServer::begin()
{
    int ret;
    if (!_pTcpServer)
    {
        Serial.println("Server unavailable");
        return;
    }

    ret = _pTcpServer->open(wiFiDrv.get_stack());
    if (ret != 0)
    {
        Serial.print("WiFiServer : socket open failed, ret=");
        Serial.println(ret);
        return;
    }

    ret = _pTcpServer->bind(_port);
    if (ret != 0)
    {
        Serial.print("WiFiServer : bind failed, ret=");
        Serial.println(ret);
        return;
    }

    ret = _pTcpServer->listen();
    if (ret != 0)
    {
        Serial.print("WiFiServer : start listening failed, ret=");
        Serial.println(ret);
        return;
    }

    Serial.print("start listening ");
    Serial.print(WiFi.localIP());
    Serial.print(": ");
    Serial.println(_port);
}

WiFiClient WiFiServer::available(byte *status)
{
    if (!_pTcpServer)
    {
        Serial.println("wifi server unavailable");
    }
    if (_currentClient)
    {
        Serial.println("there was a client");
        return *_currentClient;
    }
    return WiFiClient();
}

int WiFiServer::accept(WiFiClient *client)
{
    if (!_pTcpServer)
    {
        return -1;
    }
    int ret = _pTcpServer->accept(client->_pTcpSocket);
    _currentClient = client;
    return ret;
}

size_t WiFiServer::write(uint8_t b)
{
    return write(&b, 1);
}

size_t WiFiServer::write(const uint8_t *buffer, size_t size)
{
    if (!_pTcpServer || !_currentClient || _currentClient->connected())
    {
        Serial.print("current client not available");
        return 0;
    }
    _currentClient->write(buffer, size);
}

void WiFiServer::close()
{
    if (!_pTcpServer)
    {
        return;
    }
    _currentClient = nullptr;
    _pTcpServer->close();
}