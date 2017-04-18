/*
  WiFiServer.h - Library for Arduino Wifi shield.
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

#ifndef wifiserver_h
#define wifiserver_h

#include "Arduino.h"
#include "TCPServer.h"
#include "Server.h"
#include "WiFiClient.h"

class WiFiServer : public Server
{
private:
  uint16_t _port;
  TCPServer *_pTcpServer;
  bool _debug;
  WiFiClient *_currentClient;

public:
  WiFiServer(uint16_t, bool debug = false);
  ~WiFiServer();
  WiFiClient available(uint8_t *status = NULL);
  void begin();
  int accept(WiFiClient *client);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  void close();
  void send(int code, char *content_type, const String &content);

  using Print::write;
};

#endif
