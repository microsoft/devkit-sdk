/*
  WiFiClient.cpp - Library for Arduino Wifi shield.
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
#include "AZ3166WiFiClient.h"
#include "SystemWiFi.h"

WiFiClient::WiFiClient()
{
    _pTcpSocket = NULL;
}

WiFiClient::~WiFiClient()
{
    stop();
}

int WiFiClient::peek() 
{
    return 0;
}

int WiFiClient::connect(const char* host, uint16_t port) 
{
    if (_pTcpSocket != NULL)
    {
        // Already connected
        return 0;
    }
    
    _pTcpSocket = new TCPSocket();
    if (_pTcpSocket == NULL)
    {
        return 0;
    }
    if (_pTcpSocket->open(WiFiInterface()) != 0 || _pTcpSocket->connect(host, port) != 0)
    {
        delete _pTcpSocket;
        _pTcpSocket = NULL;
        return 0;
    }
    
    _pTcpSocket->set_blocking(false);
    _pTcpSocket->set_timeout(1000);
    return 1;
}

int WiFiClient::connect(IPAddress ip, uint16_t port) 
{
    return connect(ip.get_address(), port);
}

int WiFiClient::available() 
{
    return connected();
}

size_t WiFiClient::write(uint8_t b) 
{
    return write(&b, 1);
}

size_t WiFiClient::write(const uint8_t *buf, size_t size) 
{
    if (_pTcpSocket != NULL)
    {
        return _pTcpSocket->send((void*)buf, (int)size);
    }
    return 0;
}

int WiFiClient::read()
{
    uint8_t ch;
    
    int ret = read(&ch, 1);
    if (ret == 0)
    {
        // Connection closed
        stop();
    }
    if ( ret <= 0 ) 
        return -1;
    else 
        return (int)ch;
}

int WiFiClient::read(uint8_t* buf, size_t size) 
{
    if (_pTcpSocket != NULL)
    {
        return _pTcpSocket->recv((void*)buf, (int)size);
    }
    return -1;
}

void WiFiClient::flush() 
{

}

void WiFiClient::stop() 
{
    if (_pTcpSocket != NULL)
    {
        _pTcpSocket->close();
        delete _pTcpSocket;
        _pTcpSocket = NULL;
    }
}

uint8_t WiFiClient::connected()
{
    return ( _pTcpSocket == NULL ) ? 0 : 1;
}

WiFiClient::operator bool() 
{
    return ( _pTcpSocket != NULL ) ;
}
