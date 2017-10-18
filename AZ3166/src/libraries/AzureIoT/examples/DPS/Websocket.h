/**
* @author Samuel Mokrani
*
* @section LICENSE
*
* Copyright (c) 2011 mbed
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @section DESCRIPTION
*    Simple websocket client
*
*/

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "mbed.h"

/** Websocket client Class.
 *
 * Example (ethernet network):
 * @code
 * #include "mbed.h"
 * #include "EthernetInterface.h"
 * #include "Websocket.h"
 *
 * int main() {
 *    EthernetInterface eth;
 *    eth.init(); //Use DHCP
 *    eth.connect();
 *    printf("IP Address is %s\n\r", eth.getIPAddress());
 *   
 *    Websocket ws("ws://sockets.mbed.org:443/ws/demo/rw");
 *    ws.connect();
 *   
 *    while (1) {
 *        int res = ws.send("WebSocket Hello World!");
 *
 *        if (ws.read(recv)) {
 *            printf("rcv: %s\r\n", recv);
 *        }
 *
 *        wait(0.1);
 *    }
 * }
 * @endcode
 */
 
class Websocket
{
    public:
        /**
        * Constructor
        *
        * @param url The Websocket url in the form "ws://ip_domain[:port]/path" (by default: port = 80)
        */
        Websocket(char * url);

        /**
        * Connect to the websocket url
        *
        *@return true if the connection is established, false otherwise
        */
        bool connect();

        /**
        * Send a string according to the websocket format (see rfc 6455)
        *
        * @param str string to be sent
        *
        * @returns the number of bytes sent
        */
        int send(char * str, int size = 0, char opcode = 0x81, int mask = 0);

        /**
        * Read a websocket message
        *
        * @param message pointer to the string to be read (null if drop frame)
        *
        * @return true if a websocket frame has been read
        */
        bool read(char * message, int * length, unsigned char * opcode, bool first = true);

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
        char* getPath();

    private:
        void fillFields(char * url);
        int parseURL(const char* url, char* scheme, size_t maxSchemeLen, char* host, size_t maxHostLen, uint16_t* port, char* path, size_t maxPathLen); //Parse URL
        int sendLength(uint32_t len, char * msg);
        int sendMask(char * msg);
        int readChar(char * pC, bool block = true);
        
        char scheme[8];
        uint16_t port;
        char host[64];
        char path[64];
        
        TCPSocket socket;

        int read(char * buf, int len, int min_len = -1);
        int write(char * buf, int len);
};

#endif
