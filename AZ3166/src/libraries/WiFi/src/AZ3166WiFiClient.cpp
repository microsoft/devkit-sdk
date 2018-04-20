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
#include "AZ3166WiFiClient.h"
#include "SystemWiFi.h"

WiFiClient::WiFiClient()
{
    _pTcpSocket = NULL;
    _useServerSocket = false;
}

WiFiClient::WiFiClient(TCPSocket* socket)
{
    _pTcpSocket = socket;
    _useServerSocket = true;
}

WiFiClient::~WiFiClient()
{
    stop();
}

int WiFiClient::peek() 
{
    return 0;
}

int WiFiClient::connect(const char* host, unsigned short port) 
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
    if (_pTcpSocket->open(WiFiInterface()) != 0 || _pTcpSocket->connect(host, (uint16_t)port) != 0)
    {
        delete _pTcpSocket;
        _pTcpSocket = NULL;
        return 0;
    }
    
    _pTcpSocket->set_blocking(false);
    _pTcpSocket->set_timeout(1000);
    return 1;
}

int WiFiClient::connect(IPAddress ip, unsigned short port) 
{
    return connect(ip.get_address(), (uint16_t)port);
}

int WiFiClient::available() 
{
    return connected();
}

unsigned int WiFiClient::write(unsigned char b) 
{
    return write(&b, 1);
}

unsigned int WiFiClient::write(const unsigned char *buf, unsigned int size) 
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

int WiFiClient::read(unsigned char* buf, unsigned int size) 
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
        if(!_useServerSocket)
        {
            delete _pTcpSocket;
            _pTcpSocket = NULL;            
        }
    }
}

int WiFiClient::connected()
{
    return ( _pTcpSocket == NULL || _pTcpSocket -> send(NULL, 0) == NSAPI_ERROR_NO_SOCKET) ? 0 : 1;
}

WiFiClient::operator bool() 
{
    return ( _pTcpSocket != NULL ) ;
}
