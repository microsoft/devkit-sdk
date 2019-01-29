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
 
#ifndef wifiserver_h
#define wifiserver_h

#include "Arduino.h"
#include "TCPServer.h"
#include "AZ3166WiFiClient.h"
#include "Print.h"

class WiFiServer : public Print
{
public:
    WiFiServer(unsigned short port);
    virtual ~WiFiServer();

    WiFiClient available();
    void begin();
    void close();
    void send(int code, char *content_type, const String &content);
    virtual size_t write(unsigned char);
    virtual size_t write(const unsigned char *buf, size_t size);
  
private:
	unsigned short _port;
    TCPServer *_pTcpServer;
    TCPSocket _clientTcpSocket;
};

#endif // wifiserver_h

