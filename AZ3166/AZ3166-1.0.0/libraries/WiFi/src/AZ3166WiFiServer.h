#ifndef wifiserver_h
#define wifiserver_h

#include "Arduino.h"
#include "TCPServer.h"
#include "AZ3166WiFiClient.h"
#include "Print.h"

class WiFiServer : public Print
{
public:
    WiFiServer(uint16_t port);
    ~WiFiServer();

    WiFiClient available(uint8_t *status = NULL);
    void begin();
    int accept(WiFiClient *client);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    void close();
    void send(int code, char *content_type, const String &content);
  
private:
	uint16_t _port;
    TCPServer *_pTcpServer;
    WiFiClient _currentClient;
};

#endif // wifiserver_h

