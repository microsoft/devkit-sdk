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
 
extern "C" {
extern int rtl_printf(const char *fmt, ...);
}
#include <string.h>
#include "AZ3166WiFi.h"
#include "AZ3166WiFiUdp.h"
#include "AZ3166WiFiClient.h"
#include "SystemWiFi.h"
//#include "WiFiServer.h"

/* Constructor */
WiFiUDP::WiFiUDP() 
{
    _pUdpSocket = new UDPSocket();
    
    _localPort = 0;
    is_initialized = false;
}

/* Start WiFiUDP socket, listening at local port PORT */
int WiFiUDP::begin(unsigned short port) {

    if ( !is_initialized ) 
    {
        _pUdpSocket->set_blocking(false);  
        _pUdpSocket->set_timeout(5000);
        if(_pUdpSocket->open(WiFiInterface()) != 0)
        {
            return 0;
        }
        is_initialized = true;
    }
    
    if ( is_initialized )
    {
        _localPort = port;
        _pUdpSocket->bind(port);
        return 1;
    }
    else
    {
        return 0;
    }
}


/* Release any resources being used by this WiFiUDP instance */
void WiFiUDP::stop()
{
    if (!is_initialized)
        return;

    _pUdpSocket->close();
    is_initialized = false;
}

int WiFiUDP::beginPacket(const char *host, unsigned short port)
{
    // Look up the host first
    SocketAddress outEndPoint(host, port);
    if(WiFiInterface()->gethostbyname(host, &outEndPoint))
    {
        return 0;
    }
    IPAddress remote_addr;
    remote_addr.fromString(outEndPoint.get_ip_address());
    return beginPacket(remote_addr, port);
}

int WiFiUDP::beginPacket(IPAddress ip, unsigned short port)
{
    if ( !is_initialized ) 
    {
        if(_pUdpSocket->open(WiFiInterface()) != 0)
        {
            return 0;
        }
        
        _pUdpSocket->set_blocking(false);  
        _pUdpSocket->set_timeout(5000);

        is_initialized = true;
    }
    
    if ( is_initialized )
    {
        _address = new SocketAddress(ip.get_address(), port);
    }   
    return 1;       
}

int WiFiUDP::endPacket()
{
    return true;
}

size_t WiFiUDP::write(unsigned char data)
{
    return write(&data, 1);
}

size_t WiFiUDP::write(const unsigned char *buffer, size_t size)
{
    if ( !is_initialized ) 
        return 0;

    _pUdpSocket->sendto(*_address, (char*)buffer, size);
    return size;
}

int WiFiUDP::read()
{
    int n;
    char b;

    if (_address == NULL){
        _address = new SocketAddress();
    }
    n =  _pUdpSocket->recvfrom(_address,  &b, 1);
    return ( n != 1 )? (-1) : (int)(b);
}

int WiFiUDP::read(unsigned char* buffer, size_t len)
{
    if (_address == NULL){
        _address = new SocketAddress();
    }
    return _pUdpSocket->recvfrom(_address, (char*)buffer, len);
}

void WiFiUDP::flush()
{

}

IPAddress WiFiUDP::remoteIP()
{
    if(_address == NULL)
    {
        return IP_ADDR_NONE;
    }
    IPAddress ip;
    ip.fromString(_address->get_ip_address());
    return ip;
}

unsigned short  WiFiUDP::remotePort()
{
    if(_address == NULL)
        return 0;
    return _address->get_port();
}

WiFiUDP:: ~WiFiUDP()
{
    if(_pUdpSocket != NULL)
    {
        _pUdpSocket->close();
        delete _pUdpSocket;
    }

    if(_address != NULL)
    {
        delete _address;
    }
    is_initialized = false;
}
