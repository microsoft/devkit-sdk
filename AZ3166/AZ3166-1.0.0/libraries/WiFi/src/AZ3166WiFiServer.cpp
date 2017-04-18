/*
  A3166WiFiClient.cpp
  Copyright (c) 2011-2014 Arduino.  All right reserved.

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

#include "AZ3166WiFi.h"
#include "SystemWiFi.h"
#include "IPAddress.h"

WiFiServer::WiFiServer(uint16_t port)
{
    _port = port;
    _currentClient = NULL;
    _pTcpServer = new TCPServer();
}

WiFiServer::~WiFiServer()
{
    if (_currentClient != NULL)
    {
        delete _currentClient;
        _currentClient = NULL;
    }
    if (_pTcpServer != NULL)
    {
        delete _pTcpServer;
        _pTcpServer = NULL;
    }
}

void WiFiServer::begin()
{
    int ret;
    
    if (!_pTcpServer)
    {
        return;
    }
    
    ret = _pTcpServer->open(WiFiInterface());
    if (ret != 0)
    {
        return;
    }

    ret = _pTcpServer->bind(_port);
    if (ret != 0)
    {
        return;
    }

    ret = _pTcpServer->listen();
    if (ret != 0)
    {
        return;
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