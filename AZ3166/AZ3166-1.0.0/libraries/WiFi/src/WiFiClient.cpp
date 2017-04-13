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

#include "AzureBoardWiFi.h"
#include "WiFiClient.h"
#include "utility\wifi_drv.h"


WiFiClient::WiFiClient()  : _sock(MAX_SOCK_NUM), _isConnected(false)
{
    _pTcpSocket = new TCPSocket();
}

int WiFiClient::peek() {
	return 0;
}

int WiFiClient::connect(const char* host, uint16_t port) 
{
    int ret;
    _sock = getFirstSocket();

    if (_sock != SOCK_NOT_AVAIL)
    {
        ret = _pTcpSocket->open(&wiFiDrv);
        if ( ret != 0 ) 
        {
			      Serial.print("WiFiClient : socket open failed ret=");
			      Serial.println(ret);
            _isConnected = false;
			      return 0; // socket connect failed. 
		    }
        ret = _pTcpSocket->connect(host, port);
        if ( ret != 0 ) 
        {
			      Serial.print("WiFiClient : connect failed ret=");
			      Serial.println(ret);
            _isConnected = false;
			      return 0; // socket connect failed. 
		    }
        _isConnected = true;
        WiFiClass::_state[_sock] = _sock;
    }
    else
    { 
    	  Serial.println("No Socket available");
    	  return 0;
    }
    return 1;
}

int WiFiClient::connect(IPAddress ip, uint16_t port) 
{
	   return connect(ip.get_address(), port);
}

int WiFiClient::available() {
	int ret;

	if (_sock == 255) 
      return 0;

	if (_isConnected == false ) 
      return 0;
  else
      return 1;
	
}

size_t WiFiClient::write(uint8_t b) 
{
	  return write(&b, 1);
}

size_t WiFiClient::write(const uint8_t *buf, size_t size) 
{
    int i;  
    return _pTcpSocket->send((void*)buf, (int)size);
}


int WiFiClient::read() {
	uint8_t ch;
	int ret;
	
	ret = read(&ch, 1);
	if ( ret <= 0 ) return -1;
	else return (int)ch;
	
}


int WiFiClient::read(uint8_t* buf, size_t size) {
  return _pTcpSocket->recv((void*)buf, (int)size);
}


void WiFiClient::flush() {
  while (available())
    read();
}

void WiFiClient::stop() {

  if (_sock == 255)
    return;

  _pTcpSocket->close();
  WiFiClass::_state[_sock] = NA_STATE;

  _sock = 255;
}

uint8_t WiFiClient::connected() {

  if (_sock == 255) {
    return 0;
  } else {
  	return ( _isConnected == true )? 1 : 0;
  }
}

WiFiClient::operator bool() {
  if ( _sock == 255 ) 
      return false;
  return _isConnected;
}

// Private Methods
uint8_t WiFiClient::getFirstSocket()
{
    for (int i = 0; i < MAX_SOCK_NUM; i++) {
      if (WiFiClass::_state[i] == NA_STATE)
      {
          return i;
      }
    }
    return SOCK_NOT_AVAIL;
}

