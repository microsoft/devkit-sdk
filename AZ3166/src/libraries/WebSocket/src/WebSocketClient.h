// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include "mbed.h"
#include "SystemWiFi.h"
#include "http_common.h"
#include "http_parsed_url.h"

#define _WS_DEBUG

#ifdef _WS_DEBUG
#define INFO_FORMAT(format, args...) do {char buf[250];sprintf(buf, format, args);printf(buf);} while(0);
#define ERROR_FORMAT(format, args...) do {char buf[250];sprintf(buf, format, args);printf(buf);} while(0);

#define INFO(x) do {  } while(0);
#define ERROR(x) do {  } while(0);
#else
#define INFO_FORMAT(format, args...) do {} while(0);
#define ERROR_FORMAT(format, args...) do {} while(0);

#define INFO(x) do {  } while(0);
#define ERROR(x) do {  } while(0);
#endif

typedef enum
{
    WS_Message_Text = 0,        /* The message is clear text. */
    WS_Message_Binary,          /* The message is in binary format. */
    WS_Close                    /* A receive has completed because a close message was received. */
} MessageType;

typedef enum
{
    WS_OPCODE_CONT = 0x00,      /* denotes a continuation frame */
    WS_OPCODE_TEXT = 0x01,      /* denotes a text frame */
    WS_OPCODE_BINARY = 0x02,    /* denotes a binary frame */
    WS_OPCODE_CLOSE = 0x08,     /* denotes a connection close */
    WS_OPCODE_PING = 0x09,      /* denotes a ping */
    WS_OPCODE_PONG = 0x0A       /* denotes a pong */
};

typedef struct
{
    int length;
    bool isEndOfMessage;
    MessageType messageType;
} WebSocketReceiveResult;

class WebSocketClient
{
    public:
        /**
        * Constructor
        *
        * @param url The Websocket url in the form "ws://ip_domain[:port]/path" (by default: port = 80)
        */
        WebSocketClient(char * url);

        /**
        * Destructor
        * 
        */
        ~WebSocketClient();

        /**
        * Connect to the websocket url
        *
        *@return true if the connection is established, false otherwise
        */
        bool connect();

        /**
        * Check if the WebSocket connection is available
        *
        * @return true if the connection is available
        */
        bool connected();

        /**
        * Send a string according to the websocket format (see rfc 6455)
        *
        * @param str string to be sent
        *
        * @returns the number of bytes sent
        */
        int send(char * str, int size);

        /**
        * Read a websocket message
        *
        * @param message pointer to the string to be read (null if drop frame)
        *
        * @return true if a websocket frame has been read
        */
        WebSocketReceiveResult* receive(char *message);

        /**
        * Close the websocket connection
        *
        * @return true if the connection has been closed, false otherwise
        */
        bool close();

        /*
        * Accessor: get path from the websocket url
        *
        * @return path
        */
        const char* getPath();

    private:
        void fillFields(char * url);
        int parseURL(const char* url, char* scheme, size_t maxSchemeLen, char* host, size_t maxHostLen, uint16_t* port, char* path, size_t maxPathLen);
        int sendLength(uint32_t len, char * msg);
        int sendMask(char * msg);
        int readChar(char * pC, bool block = true);

        int read(char * buf, int len, int min_len = -1);
        int write(char * buf, int len);

        TCPSocket * _tcpSocket;
        ParsedUrl * _parsedUrl;
        uint16_t _port;
        MessageType messageType;
};

#endif
