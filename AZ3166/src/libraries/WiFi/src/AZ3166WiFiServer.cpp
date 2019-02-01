/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AZ3166WiFi.h"
#include "AZ3166WiFiServer.h"
#include "SystemWiFi.h"
#include "IPAddress.h"

WiFiServer::WiFiServer(unsigned short port)
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

WiFiClient WiFiServer::available()
{
    if (_pTcpServer == NULL )
    {
        return WiFiClient();
    }

    int ret = _pTcpServer->accept(&_clientTcpSocket);
    if (ret == 0)
    {
        return WiFiClient(&_clientTcpSocket);
    }
    else
    {
        return WiFiClient();
    }
}

size_t WiFiServer::write(unsigned char b)
{
    return write(&b, 1);
}

size_t WiFiServer::write(const unsigned char *buffer, size_t size)
{
    return this->_clientTcpSocket.send((void*)buffer, size);
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