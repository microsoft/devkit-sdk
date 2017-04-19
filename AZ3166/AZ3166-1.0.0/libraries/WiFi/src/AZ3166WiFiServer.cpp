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
#include "AZ3166WiFiServer.h"
#include "SystemWiFi.h"
#include "IPAddress.h"

WiFiServer::WiFiServer(uint16_t port)
{
    _port = port;
    _pTcpServer = NULL;
}

WiFiServer::~WiFiServer()
{
    close();
}

void WiFiServer::begin()
{
    int ret;
    
    if (_pTcpServer != NULL)
    {
        return;
    }
    
    _pTcpServer = new TCPServer();
    if(_pTcpServer == NULL)
    {
        return;
    }
    
    if (_pTcpServer->open(WiFiInterface()) != 0)
    {
        delete _pTcpServer;
        _pTcpServer = NULL;
        return;
    }
    if (_pTcpServer->bind(_port) != 0 || _pTcpServer->listen() != 0)
    {
        _pTcpServer->close();
        delete _pTcpServer;
        _pTcpServer = NULL;
        return;
    }
}

WiFiClient WiFiServer::available(byte *status)
{
    WiFiClient tmpClient;
    accept(&tmpClient);
    return tmpClient;
}

int WiFiServer::accept(WiFiClient *client)
{
    if (_pTcpServer == NULL || client == NULL || client->_pTcpSocket != NULL)
    {
        return -1;
    }
    int ret = _pTcpServer->accept(client->_pTcpSocket);
    if (ret == 0)
    {
        _currentClient._pTcpSocket = client->_pTcpSocket;
    }
    return ret;
}

size_t WiFiServer::write(uint8_t b)
{
    return write(&b, 1);
}

size_t WiFiServer::write(const uint8_t *buffer, size_t size)
{
    _currentClient.write(buffer, size);
}

void WiFiServer::close()
{
    if (_pTcpServer == NULL)
    {
        return;
    }
    _pTcpServer->close();
    delete _pTcpServer;
    _pTcpServer = NULL;
}