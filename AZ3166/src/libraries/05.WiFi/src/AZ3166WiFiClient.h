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
 
#ifndef wificlient_h
#define wificlient_h

#include "Arduino.h"	
#include "IPAddress.h"
#include "TCPSocket.h"
#include "Print.h"

class WiFiClient : public Print
{
public:
  WiFiClient(TCPSocket* socket);
  WiFiClient();
  ~WiFiClient();

  virtual int connect(IPAddress ip, unsigned short port);
  virtual int connect(const char *host, unsigned short port);
  virtual unsigned int write(unsigned char);
  virtual unsigned int write(const unsigned char *buf, unsigned int size);
  virtual int available();
  virtual int read();
  virtual int read(unsigned char *buf, unsigned int size);
  virtual void flush();
  virtual void stop();
  virtual int connected();
  virtual operator bool();
  virtual int peek();
  
  friend class WiFiServer;

private:
  TCPSocket* _pTcpSocket;
  bool _useServerSocket;
};

#endif

