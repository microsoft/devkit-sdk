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


WiFiServer::WiFiServer(uint16_t port, bool debug)
{
    close();
    _port = port;
    _currentClient = NULL;
    _debug = debug;
    _pTcpServer = new TCPServer();
}
WiFiServer::~WiFiServer()
{
    delete _currentClient;
    delete _pTcpServer;
}

void WiFiServer::begin()
{
    int ret;
    if (!_pTcpServer)
    {
        if(_debug)
        {
            printf("Server unavailable");
        }
        return;
    }

    ret = _pTcpServer->open(wiFiDrv.get_stack());
    if (ret != 0)
    {
        if(_debug)
        {
            printf("WiFiServer : socket open failed, ret = %d\r\n", ret);
        }
        return;
    }

    ret = _pTcpServer->bind(_port);
    if (ret != 0)
    {
        if(_debug)
        {
            printf("WiFiServer : tcp bind failed, ret = %d\r\n", ret);
        }
        return;
    }

    ret = _pTcpServer->listen();
    if (ret != 0)
    {
        if(_debug)
        {
            printf("WiFiServer : tcp server listen failed, ret = %d\r\n", ret);
        }
        return;
    }
    if(_debug)
    {
        printf("start listening %s on port %d\r\n", WiFi.localIP(), _port);
    }
}

WiFiClient WiFiServer::available(byte *status)
{
    if (_currentClient && _currentClient->read() != -1)
    {
        return *_currentClient;
    }
    WiFiClient _newClient;
    return _newClient;
}

int WiFiServer::accept(WiFiClient *client)
{
    if (!_pTcpServer || !client || !client->_pTcpSocket)
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
    _pTcpServer->close();
}