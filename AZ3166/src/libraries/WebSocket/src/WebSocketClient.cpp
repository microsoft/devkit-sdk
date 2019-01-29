#include "WebSocketClient.h"

#define MAX_TRY_WRITE 30
#define MAX_TRY_READ 10

// Currently we used a pre-calculated pair of based-64 strings for WebSocket opening handshake.
// TODO: used a random generated key in client side and calculate the valid server response string
// according to the WebSockeet protocol defenition (see https://tools.ietf.org/html/rfc6455#section-1.3)
const char WS_HANDSHAKE_CLIENT_KEY[] = "L159VM0TWUzyDxwJEIEzjw==";
const char WS_HANDSHAKE_SERVER_ACCEPT[] = "DdLWT/1JcX+nQFHebYP+rqEx5xI=";

static WebSocketReceiveResult receiveResult;

WebSocketClient::WebSocketClient(char *url)
{
    _tcpSocket = NULL;
    _parsedUrl = NULL;
    _parsedUrl = new ParsedUrl(url);
    _firstFrame = true;

    if (!_parsedUrl->schema())
    {
        if (strcmp(_parsedUrl->schema(), "ws") == 0)
        {
            _port = _parsedUrl->port();
        }
        else
        {
            // TODO: support secure WebSocket (port 443)
            _port = 443;
        }
    }
}

WebSocketClient::~WebSocketClient()
{
    if (_tcpSocket != NULL)
    {
        _tcpSocket->close();
        delete _tcpSocket;
        _tcpSocket = NULL;
    }

    if (_parsedUrl != NULL)
    {
        delete _parsedUrl;
        _parsedUrl = NULL;
    }
}

bool WebSocketClient::connect(int timeout)
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

        if (_tcpSocket->open(WiFiInterface()) != 0 || _tcpSocket->connect(_parsedUrl->host(), _parsedUrl->port()) != 0)
        {
            ERROR_FORMAT("Unable to connect to %s on port %d\r\n", _parsedUrl->host(), _parsedUrl->port());

            delete _tcpSocket;
            _tcpSocket = NULL;
            return false;
        }
        _tcpSocket->set_blocking(true);
        _tcpSocket->set_timeout(TIMEOUT_IN_MS);
    }

    return doHandshake(timeout);
}

bool WebSocketClient::doHandshake(int timeout)
{
    char strBuffer[250];

    // Send handshake msgBuffer to upgrade http to the ws protocol
    if (strlen(_parsedUrl->query()) > 0)
    {
        sprintf(strBuffer, "GET %s?%s HTTP/1.1\r\n", _parsedUrl->path(), _parsedUrl->query());
    }
    else
    {
        sprintf(strBuffer, "GET %s HTTP/1.1\r\n", _parsedUrl->path());
    }
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Host: %s:%d\r\n", _parsedUrl->host(), _parsedUrl->port());
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Upgrade: WebSocket\r\n");
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Connection: Upgrade\r\n");
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Sec-WebSocket-Key: %s\r\n", WS_HANDSHAKE_CLIENT_KEY);
    write(strBuffer, strlen(strBuffer));

    sprintf(strBuffer, "Sec-WebSocket-Version: 13\r\n\r\n");
    int ret = write(strBuffer, strlen(strBuffer));
    if (ret != (int)strlen(strBuffer))
    {
        close();
        ERROR("Could not send request.");
        return false;
    }

    // Receive handshake response from WebSocket server
    ret = _tcpSocket->recv(strBuffer, 250);
    if (ret > 0)
    {
        strBuffer[ret] = '\0';

        // Server accepted the client handshake
        if (strstr(strBuffer, WS_HANDSHAKE_SERVER_ACCEPT) != NULL)
        {
            return true;
        }
        else
        {
            ERROR("Server didn't accept the client handshake.");
            return false;
        }
    }

    ERROR_FORMAT("Received handshake response from server failed. Return value: %d", ret);
    return false;
}

bool WebSocketClient::connected()
{
    return (_tcpSocket == NULL) ? false : true;
}

int WebSocketClient::sendLength(long len, char *msg)
{
    // According to rfc6455: https://tools.ietf.org/html/rfc6455
    // If the length of the "payload data" in bytes is 0-125, then that is the payload length.
    if (len < 126)
    {
        msg[0] = len | (1 << 7);
        return 1;
    }
    // if 126, then the following 2 bytes interpreted as a
    // 16-bit unsigned integer are the payload length
    else if (len < 65535)
    {
        msg[0] = 126 | (1 << 7);
        msg[1] = (len >> 8) & 0xff;
        msg[2] = len & 0xff;
        return 3;
    }
    // If 127, the following 8 bytes interpreted as a 64-bit unsigned integer
    // (the most significant bit MUST be 0) are the payload length.
    else
    {
        INFO_FORMAT("sendLength for length %d", len);
        msg[0] = 127 | (1 << 7);
        msg[1] = 0;
        msg[2] = 0;
        msg[3] = 0;
        msg[4] = 0;
        msg[5] = (len >> 24) & 0xff;
        msg[6] = (len >> 16) & 0xff;
        msg[7] = (len >> 8) & 0xff;
        msg[8] = len & 0xff;
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

int WebSocketClient::send(const char *str, long size, WS_Message_Type messageType, bool isFinal)
{
    if (_tcpSocket == NULL)
    {
        ERROR("unable to send data when WebSocket is disconnected.");
        return NSAPI_ERROR_NO_SOCKET;
    }

    if (str == NULL || size <= 0)
    {
        return 0;
    }

    char msg[15];

    char opcode = 0x00;
    if (messageType == WS_Message_Ping) 
    {
        opcode = WS_OPCODE_PING | WS_FINAL_BIT;
    }
    else if (messageType == WS_Message_Pong)
    {
        opcode = WS_OPCODE_PONG | WS_FINAL_BIT;
    }
    else if (messageType == WS_Message_Close)
    {
        opcode = WS_OPCODE_CLOSE | WS_FINAL_BIT;
    }
    else
    {
        if (_firstFrame)
        {
            _messageType = messageType;
            if (messageType == WS_Message_Text)
            {
                opcode = WS_OPCODE_TEXT;
            }
            else if (messageType == WS_Message_Binary)
            {
                opcode = WS_OPCODE_BINARY;
            }
        }
        else
        {
            opcode = WS_OPCODE_CONT;
        }

        if (isFinal)
        {
            opcode |= WS_FINAL_BIT;

            // Reset the states of next message to send
            _firstFrame = true;
        }
        else
        {
            // Next frame will be a continuation frame
            _firstFrame = false;
        }
    }

    msg[0] = opcode;

    int idx = 1;
    idx += sendLength(size, msg + idx);
    idx += sendMask(msg + idx);
    int res = write(msg, idx);
    if (res != idx)
    {
        ERROR("Send websocket frame header failed.");
        return -1;
    }

    res = write(str, size);
    return res == -1 ? -1 : res + idx;
}

int WebSocketClient::sendPing(char * str, int size)
{
    return send(str, size, WS_Message_Ping);
}

WebSocketReceiveResult *WebSocketClient::receive(char *msgBuffer, int size, int timeout)
{
    if (_tcpSocket == NULL)
    {
        ERROR("Unable to receive data when WebSocket is disconnected.");
        return NULL;
    }

    if (msgBuffer == NULL || size <= 0)
    {
        ERROR("Invalid message buffer to be read in WebSocket.");
        return NULL;
    }

    int i = 0;
    uint32_t payloadLength;
    char recvByte = 0;
    char opcode = 0;
    char c;
    char mask[4] = {0, 0, 0, 0};
    bool isMasked = false;
    bool isFinal = false;
    Timer timer;

    receiveResult.isEndOfMessage = true;
    receiveResult.length = 0;
    receiveResult.messageType = WS_Message_Text;

    // Read opcode
    timer.start();
    _tcpSocket->set_timeout(timeout);
    while (true)
    {
        if (timer.read_ms() > timeout)
        {
            // A timeout is not an error when you are polling
            INFO("WebSocket receive timeout");
            receiveResult.messageType = WS_Message_Timeout;        
            return &receiveResult;
        }

        int res = _tcpSocket->recv(&recvByte, 1);
        if (res == 1)
        {
            opcode = recvByte & 0x7F;

            // opcode for data frames
            if (opcode == WS_OPCODE_CONT || opcode == WS_OPCODE_TEXT || opcode == WS_OPCODE_BINARY)
            {
                if (opcode == WS_OPCODE_TEXT)
                {
                    _messageType = WS_Message_Text;
                }
                else if (opcode == WS_OPCODE_BINARY)
                {
                    _messageType = WS_Message_Binary;
                }

                isFinal = ((recvByte & 0x80) == 0x80);
                break;
            }
            // opcode for connection close
            else if (opcode == WS_OPCODE_CLOSE)
            {
                INFO("received close");
                _messageType = WS_Message_Close;        
                break;
            }
            else if (opcode == WS_OPCODE_PING)
            {
                INFO("received ping");
                _messageType = WS_Message_Ping;
                break;
            }
            else if (opcode == WS_OPCODE_PONG)
            {
                INFO("received pong");
                _messageType = WS_Message_Pong;
                break;
            }
        }
        else if (res < 0 && res != NSAPI_ERROR_WOULD_BLOCK)
        {
            ERROR_FORMAT("Socket receive failed, res: %d, opcode: %x\r\n", res, opcode);
            if (res == NSAPI_ERROR_NO_CONNECTION)
            {
                close();
            }
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
        readChar(&c); // 0
        readChar(&c); // 0
        readChar(&c); // 0
        readChar(&c); // 0
        readChar(&c);
        payloadLength = c << 24;
        readChar(&c);
        payloadLength += c << 16;
        readChar(&c);
        payloadLength += c << 8;
        readChar(&c);
        payloadLength += c;
    }
    INFO_FORMAT("Frame length:%d ismasked:%d", payloadLength, isMasked);

    if (isMasked)
    {
        INFO("Payload is masked");
        for (i = 0; i < 4; i++)
        {
            readChar(&c);
            mask[i] = c;
        }
    }

    if (payloadLength > 0)
    {
        uint32_t len = payloadLength;
        if (payloadLength > (uint32_t)size)
        {
            len = size;
        }

        int nb = read(msgBuffer, len, len);
        if (nb != (int)len) 
        {
            ERROR("read failed");
            return NULL;
        }

        if (payloadLength > (uint32_t)size)
        {
            for (i = len; i < (int)payloadLength; i += 1)
            {
                readChar(&c);
            }
            _messageType = WS_Message_BufferOverrun;
        }

        INFO("applying mask");
        for (i = 0; i < (int)len; i++)
        {
            msgBuffer[i] = msgBuffer[i] ^ mask[i % 4];
        }
        msgBuffer[len] = '\0';
    }

    if (_messageType == WS_Message_Ping)
    {
        INFO("sending pong");
        send(msgBuffer, payloadLength, WS_Message_Pong);
    }
    else if (_messageType == WS_Message_Close)
    {
        INFO("closing connection");
        close();
    }

    receiveResult.isEndOfMessage = isFinal;
    receiveResult.length = payloadLength;
    receiveResult.messageType = _messageType;  
          
    if (_messageType == WS_Message_Ping ||
        _messageType == WS_Message_Close ||
        _messageType == WS_Message_Timeout)
    {
        // For backwards compatibility with samples
        // return a length of 0 for any new message
        // types that old code could receive.
        // Since pings are automatically handled above
        // the user should not need the ping data.
        receiveResult.length = 0;
    }
    
    return &receiveResult;
}

bool WebSocketClient::close()
{
    // Send a close frame to the server to tell 
    // it the client is closing from here.
    return send("*", 1, WS_Message_Close);

    if (_tcpSocket == NULL)
    {
        INFO("WebSocket is already disconnected.");
        return true;
    }

    int ret = _tcpSocket->close();
    if (ret < 0)
    {
        ERROR("Could not disconnect to WebSocket server.");
        return false;
    }

    delete _tcpSocket;
    _tcpSocket = NULL;
    return true;
}

const char *WebSocketClient::getPath()
{
    return _parsedUrl->path();
}

int WebSocketClient::write(const char *str, int len)
{
    int res = 0, idx = 0;

    for (int j = 0; j < MAX_TRY_WRITE; j++)
    {
        if ((res = _tcpSocket->send(str + idx, len - idx)) < 0)
        {
            continue;
        }
        else
        {
            // reset the retry count since we sent something
            j = 0;
        }

        idx += res;

        if (idx == len)
        {
            return len;
        }

        wait_ms(40);
    }

    return (idx == 0) ? -1 : idx;
}

int WebSocketClient::read(char *str, int len, int min_len)
{
    int res = 0, idx = 0;

    for (int j = 0; j < MAX_TRY_WRITE; j++)
    {
        res = _tcpSocket->recv(str + idx, len - idx);
        if (res < 0)
        {
            continue;
        }
        else
        {
            // reset the retry count since we received something
            j = 0;
        }

        idx += res;

        if (idx == len || (min_len != -1 && idx > min_len))
            return idx;
    }

    return (idx == 0) ? -1 : idx;
}
