#include "Arduino.h"
#include "WebSocketClient.h"
#include "SystemWiFi.h"

#define MAX_TRY_WRITE 30
#define MAX_TRY_READ 10

WebSocketClient::WebSocketClient(char *url)
{
    fillFields(url);
    _tcpSocket = NULL;
}

WebSocketClient::~WebSocketClient()
{
    if (_tcpSocket != NULL)
    {
        _tcpSocket->close();
        delete _tcpSocket;
        _tcpSocket = NULL;
    }
}

void WebSocketClient::fillFields(char *url)
{
    int ret = parseURL(url, scheme, sizeof(scheme), host, sizeof(host), &port, path, sizeof(path));
    if (ret)
    {
        Serial.println("ERROR: URL parsing failed; please use: \"ws://ip-or-domain[:port]/path\".");
        return;
    }

    if (port == 0) //TODO: do handle WSS->443
    {
        port = 80;
    }

    if (strcmp(scheme, "ws"))
    {
        Serial.println("ERROR: Wrong scheme, please use \"ws\" instead.");
    }
}

int WebSocketClient::parseURL(const char *url, char *scheme, size_t maxSchemeLen, char *host, size_t maxHostLen, uint16_t *port, char *path, size_t maxPathLen)
{
    char *schemePtr = (char *)url;
    char *hostPtr = (char *)strstr(url, "://");
    if (hostPtr == NULL)
    {
        Serial.println("ERROR: Could not find host from the WebSocket URL.");
        return -1;
    }

    if (maxSchemeLen < hostPtr - schemePtr + 1) // Including NULL-terminating char
    {
        Serial.printf("ERROR: Scheme str is too small (%d >= %d)", maxSchemeLen, hostPtr - schemePtr + 1);
        return -1;
    }
    memcpy(scheme, schemePtr, hostPtr - schemePtr);
    scheme[hostPtr - schemePtr] = '\0';

    hostPtr += 3;
    size_t hostLen = 0;

    char *portPtr = strchr(hostPtr, ':');
    if (portPtr != NULL)
    {
        hostLen = portPtr - hostPtr;
        portPtr++;
        if (sscanf(portPtr, "%hu", port) != 1)
        {
            Serial.println("ERROR: Could not find port.");
            return -1;
        }
    }
    else
    {
        *port = 0;
    }
    char *pathPtr = strchr(hostPtr, '/');
    if (pathPtr == NULL)
    {
        Serial.println("ERROR: Path not specified. Please add /[path] to the end of the websocket address");
        return -1;
    }
    if (hostLen == 0)
    {
        hostLen = pathPtr - hostPtr;
    }

    if (maxHostLen < hostLen + 1) //including NULL-terminating char
    {
        Serial.printf("ERROR: Host str is too small (%d >= %d)", maxHostLen, hostLen + 1);
        return -1;
    }
    memcpy(host, hostPtr, hostLen);
    host[hostLen] = '\0';

    size_t pathLen;
    char *fragmentPtr = strchr(hostPtr, '#');
    if (fragmentPtr != NULL)
    {
        pathLen = fragmentPtr - pathPtr;
    }
    else
    {
        pathLen = strlen(pathPtr);
    }

    if (maxPathLen < pathLen + 1) //including NULL-terminating char
    {
        Serial.printf("ERROR: Path str is too small (%d >= %d)", maxPathLen, pathLen + 1);
        return -1;
    }
    memcpy(path, pathPtr, pathLen);
    path[pathLen] = '\0';

    return 0;
}

bool WebSocketClient::connect()
{
    if (_tcpSocket != NULL)
    {
        // Already connected.
        return true;
    }
    else
    {
        _tcpSocket = new TCPSocket();
        if (_tcpSocket == NULL)
        {
            return false;
        }

        if (_tcpSocket->open(WiFiInterface()) != 0 || _tcpSocket->connect(host, port) != 0)
        {
            Serial.printf("ERROR: Unable to connect to (%s) on port (%d)\r\n", host, port);
            wait(0.2);

            delete _tcpSocket;
            _tcpSocket = NULL;
            return false;
        }

        _tcpSocket->set_timeout(1000);
    }

    char cmd[200];

    // sent http header to upgrade to the ws protocol
    sprintf(cmd, "GET %s HTTP/1.1\r\n", path);
    write(cmd, strlen(cmd));

    sprintf(cmd, "Host: %s:%d\r\n", host, port);
    write(cmd, strlen(cmd));

    sprintf(cmd, "Upgrade: WebSocket\r\n");
    write(cmd, strlen(cmd));

    sprintf(cmd, "Connection: Upgrade\r\n");
    write(cmd, strlen(cmd));

    sprintf(cmd, "Sec-WebSocket-Key: L159VM0TWUzyDxwJEIEzjw==\r\n");
    write(cmd, strlen(cmd));

    sprintf(cmd, "Sec-WebSocket-Version: 13\r\n\r\n");
    int ret = write(cmd, strlen(cmd));
    if (ret != strlen(cmd))
    {
        close();
        Serial.println("ERROR: Could not send request.");
        return false;
    }

    ret = read(cmd, 200, 100);
    if (ret < 0)
    {
        close();
        Serial.println("ERROR: Could not receive answer.");
        return false;
    }

    cmd[ret] = '\0';

    if (strstr(cmd, "DdLWT/1JcX+nQFHebYP+rqEx5xI=") == NULL)
    {
        Serial.printf("ERROR: Wrong answer from server, got \"%s\" instead\r\n", cmd);
        do
        {
            ret = read(cmd, 200, 100);
            if (ret < 0)
            {
                Serial.println("ERROR: Could not receive answer.");
                return false;
            }
            cmd[ret] = '\0';
        } while (ret > 0);

        close();
        return false;
    }

    return true;
}

bool WebSocketClient::connected()
{
    return ( _tcpSocket == NULL ) ? false : true;
}

int WebSocketClient::sendLength(uint32_t len, char *msg)
{
    if (len < 126)
    {
        msg[0] = len | (1 << 7);
        return 1;
    }
    else if (len < 65535)
    {
        msg[0] = 126 | (1 << 7);
        msg[1] = (len >> 8) & 0xff;
        msg[2] = len & 0xff;
        return 3;
    }
    else
    {
        msg[0] = 127 | (1 << 7);
        for (int i = 0; i < 8; i++)
        {
            msg[i + 1] = (len >> i * 8) & 0xff;
        }
        return 9;
    }
}

int WebSocketClient::readChar(char *pC, bool block)
{
    int ret = read(pC, 1, 1);
    return ret;
}

int WebSocketClient::sendMask(char *msg)
{
    for (int i = 0; i < 4; i++)
    {
        msg[i] = 0x00;
    }
    return 4;
}

int WebSocketClient::send(char *str, int size, char opcode, int mask)
{
    char msg[15];
    int idx = 1;
    msg[0] = opcode;
    if (size == 0)
        size = strlen(str);
    idx += sendLength(size, msg + idx);
    idx += sendMask(msg + idx);
    int res = write(msg, idx);
    if (res != idx)
    {
        Serial.println("ERROR: send websocket frame header failed.");
        return -1;
    }
    res = write(str, size);
    if (res == -1)
        return -1;
    return res + idx;
}

WebSocketReceiveResult *WebSocketClient::receive(char *message)
{
    int i = 0;
    uint32_t payloadLength;
    char recvByte = 0;
    char opcode = 0;
    char c;
    char mask[4] = {0, 0, 0, 0};
    bool isMasked = false;
    bool isFinal = false;
    Timer timer;

    // Read opcode
    timer.start();
    while (true)
    {
        if (timer.read() > 10)
        {
            //Serial.printf("ERROR: timeout ws, timer: %d\r\n", timer.read());
            return NULL;
        }

        int res = _tcpSocket->recv(&recvByte, 1);

        if (res == 1)
        {
            _tcpSocket->set_timeout(2000);
            opcode = recvByte & 0x7F;

            // opcode for data frames
            if (opcode == WS_OPCODE_CONT || opcode == WS_OPCODE_TEXT || opcode == WS_OPCODE_BINARY)
            {
                if (opcode == WS_OPCODE_TEXT)
                {
                    messageType = WS_Message_Text;
                }
                else if (opcode == WS_OPCODE_BINARY)
                {
                    messageType = WS_Message_Binary;
                }

                isFinal = ((recvByte & 0x80) == 0x80);
                break;
            }
            // opcode for connection close
            else if (opcode == WS_OPCODE_CLOSE)
            {
                close();
            }
        }
        else if (res < 0 && res != NSAPI_ERROR_WOULD_BLOCK)
        {
            Serial.printf("ERROR: Socket receive failed, res: %d, opcode: %x\r\n", res, opcode);
            return NULL;
        }
    }

    // Parse payload length
    readChar(&c);
    payloadLength = c & 0x7f;
    isMasked = c & 0x80;
    if (payloadLength == 126)
    {
        readChar(&c);
        payloadLength = c << 8;
        readChar(&c);
        payloadLength += c;
    }
    else if (payloadLength == 127)
    {
        payloadLength = 0;
        for (int i = 0; i < 8; i++)
        {
            readChar(&c);
            payloadLength += (c << (7 - i) * 8);
        }
    }

    if (payloadLength == 0)
    {
        return NULL;
    }

    if (isMasked)
    {
        for (i = 0; i < 4; i++)
        {
            readChar(&c);
            mask[i] = c;
        }
    }

    int nb = read(message, payloadLength, payloadLength);
    if (nb != payloadLength)
        return NULL;

    for (i = 0; i < payloadLength; i++)
    {
        message[i] = message[i] ^ mask[i % 4];
    }

    message[payloadLength] = '\0';

    WebSocketReceiveResult *receiveResult = new WebSocketReceiveResult();
    receiveResult->isEndOfMessage = isFinal;
    receiveResult->length = payloadLength;
    receiveResult->messageType = messageType;

    return receiveResult;
}

bool WebSocketClient::close()
{
    if (_tcpSocket == NULL)
    {
        Serial.println("WebSocket is already disconnected.");
        return true;
    }

    int ret = _tcpSocket->close();
    if (ret < 0)
    {
        Serial.println("ERROR: Could not disconnect to WebSocket server.");
        return false;
    }

    delete _tcpSocket;
    _tcpSocket = NULL;
    return true;
}

char *WebSocketClient::getPath()
{
    return path;
}

int WebSocketClient::write(char *str, int len)
{
    int res = 0, idx = 0;

    for (int j = 0; j < MAX_TRY_WRITE; j++)
    {

        if ((res = _tcpSocket->send(str + idx, len - idx)) < 0)
        {
            continue;
        }

        idx += res;

        if (idx == len)
            return len;
        wait_ms(40);
    }

    return (idx == 0) ? -1 : idx;
}

int WebSocketClient::read(char *str, int len, int min_len)
{
    int res = 0, idx = 0;

    for (int j = 0; j < MAX_TRY_WRITE; j++)
    {

        if ((res = _tcpSocket->recv(str + idx, len - idx)) < 0)
            continue;

        idx += res;

        if (idx == len || (min_len != -1 && idx > min_len))
            return idx;
    }

    return (idx == 0) ? -1 : idx;
}
