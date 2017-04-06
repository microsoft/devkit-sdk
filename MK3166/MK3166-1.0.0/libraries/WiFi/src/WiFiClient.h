#ifndef wificlient_h
#define wificlient_h

#include "Arduino.h"	
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"
#include "TCPSocket.h"

class WiFiClient : public Client {

public:
	
  WiFiClient();

  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();
  virtual int peek();
  using Print::write;

private:
	TCPSocket* _pTcpSocket;
  
	uint8_t _sock;	 

	uint8_t getFirstSocket();

  bool _isConnected;

};

#endif

