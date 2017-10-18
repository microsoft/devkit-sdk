#include "Websocket.h"
#include "SystemWiFi.h"

#define MAX_TRY_WRITE 30
#define MAX_TRY_READ 10

//Debug is disabled by default
#if 0
#define DBG(x, ...) std::printf("[WebSocket : DBG]"x"\r\n", ##__VA_ARGS__); 
#define WARN(x, ...) std::printf("[WebSocket : WARN]"x"\r\n", ##__VA_ARGS__); 
#define ERR(x, ...) std::printf("[WebSocket : ERR]"x"\r\n", ##__VA_ARGS__); 
#else
#define DBG(x, ...) 
#define WARN(x, ...)
#define ERR(x, ...) 
#endif

// #define INFO(x, ...) printf("[WebSocket : INFO]"x"\r\n", ##__VA_ARGS__); 

Websocket::Websocket(char * url) {
    fillFields(url);
    socket.open(WiFiInterface());
    socket.set_timeout(400);
}

void Websocket::fillFields(char * url) {
  int ret = parseURL(url, scheme, sizeof(scheme), host, sizeof(host), &port, path, sizeof(path));
  if(ret)
  {
    ERR("URL parsing failed; please use: \"ws://ip-or-domain[:port]/path\"");
    return;
  }

  if(port == 0) //TODO do handle WSS->443
  {
    port = 80;
  }
  
  if(strcmp(scheme, "ws"))
  {
    ERR("Wrong scheme, please use \"ws\" instead");
  }
}

int Websocket::parseURL(const char* url, char* scheme, size_t maxSchemeLen, char* host, size_t maxHostLen, uint16_t* port, char* path, size_t maxPathLen) //Parse URL
{
  char* schemePtr = (char*) url;
  char* hostPtr = (char*) strstr(url, "://");
  if(hostPtr == NULL)
  {
    WARN("Could not find host");
    return -1; //URL is invalid
  }

  if( maxSchemeLen < hostPtr - schemePtr + 1 ) //including NULL-terminating char
  {
    WARN("Scheme str is too small (%d >= %d)", maxSchemeLen, hostPtr - schemePtr + 1);
    return -1;
  }
  memcpy(scheme, schemePtr, hostPtr - schemePtr);
  scheme[hostPtr - schemePtr] = '\0';

  hostPtr+=3;

  size_t hostLen = 0;

  char* portPtr = strchr(hostPtr, ':');
  if( portPtr != NULL )
  {
    hostLen = portPtr - hostPtr;
    portPtr++;
    if( sscanf(portPtr, "%hu", port) != 1)
    {
      WARN("Could not find port");
      return -1;
    }
  }
  else
  {
    *port=0;
  }
  char* pathPtr = strchr(hostPtr, '/');
  if(pathPtr == NULL)
  {
    WARN("Path not specified. Please add /[path] to the end of the websocket address");
    return -1;
  }
  if( hostLen == 0 )
  {
    hostLen = pathPtr - hostPtr;
  }

  if( maxHostLen < hostLen + 1 ) //including NULL-terminating char
  {
    WARN("Host str is too small (%d >= %d)", maxHostLen, hostLen + 1);
    return -1;
  }
  memcpy(host, hostPtr, hostLen);
  host[hostLen] = '\0';

  size_t pathLen;
  char* fragmentPtr = strchr(hostPtr, '#');
  if(fragmentPtr != NULL)
  {
    pathLen = fragmentPtr - pathPtr;
  }
  else
  {
    pathLen = strlen(pathPtr);
  }

  if( maxPathLen < pathLen + 1 ) //including NULL-terminating char
  {
    WARN("Path str is too small (%d >= %d)", maxPathLen, pathLen + 1);
    return -1;
  }
  memcpy(path, pathPtr, pathLen);
  path[pathLen] = '\0';

  return 0;
}


bool Websocket::connect() {
    char cmd[200];

    while (socket.connect(host, port) < 0) {
        printf("Unable to connect to (%s) on port (%d)\r\n", host, port);
        wait(0.2);
        return false;
    }

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
    if (ret != strlen(cmd)) {
        close();
        ERR("Could not send request");
        return false;
    }

    ret = read(cmd, 200, 100);
    if (ret < 0) {
        close();
        printf("Could not receive answer\r\n");
        return false;
    }

    cmd[ret] = '\0';
    DBG("recv: %s\r\n", cmd);

    if ( strstr(cmd, "DdLWT/1JcX+nQFHebYP+rqEx5xI=") == NULL ) {
        ERR("Wrong answer from server, got \"%s\" instead\r\n", cmd);
        do {
            ret = read(cmd, 200, 100);
            if (ret < 0) {
                ERR("Could not receive answer\r\n");
                return false;
            }
            cmd[ret] = '\0';
        } while (ret > 0);
        close();
        return false;
    }

//    INFO("\r\nhost: %s\r\npath: %s\r\nport: %d\r\n\r\n", host, path, port);
    return true;
}

int Websocket::sendLength(uint32_t len, char * msg) {

    if (len < 126) {
        msg[0] = len | (1<<7);
        return 1;
    } else if (len < 65535) {
        msg[0] = 126 | (1<<7);
        msg[1] = (len >> 8) & 0xff;
        msg[2] = len & 0xff;
        return 3;
    } else {
        msg[0] = 127 | (1<<7);
        for (int i = 0; i < 8; i++) {
            msg[i+1] = (len >> i*8) & 0xff;
        }
        return 9;
    }
}

int Websocket::readChar(char * pC, bool block) {
    int ret = read(pC, 1, 1);
    return ret;
}

int Websocket::sendMask(char * msg) {
    for (int i = 0; i < 4; i++) {
        msg[i] = 0x00;
    }
    return 4;
}

int Websocket::send(char * str, int size, char opcode, int mask) {
    char msg[15];
    int idx = 1;
    msg[0] = opcode;
    if (size == 0) size = strlen(str);
    idx += sendLength(size, msg + idx);
    idx += sendMask(msg + idx);
    int res = write(msg, idx);
    if (res != idx) {
        printf("send websocket frame header failed\r\n");
        return -1;
    }
    res = write(str, size);
    if (res == -1) return -1;
    return res + idx;
}

bool Websocket::read(char * message, int * length, unsigned char * opcode, bool first) {
    int i = 0;
    uint32_t len_msg;
    char c;
    char mask[4] = {0, 0, 0, 0};
    bool is_masked = false;
    Timer tmr;

    // read the opcode
    tmr.start();
    while (true) {
        if (tmr.read() > 20) {
            printf("\r\ntimeout ws\r\n");
            return false;
        }
        int res = socket.recv(opcode, 1);
        if (res == 0) continue;
        if (res < 0 && res != -3001) {
            printf("recv not 1 %d %d\r\n", res, *opcode);
            return false;
        }
        if (first && ((*opcode & 0x7F) == 0x01 || (*opcode & 0x7F) == 0x02))
            break;
        if (!first && (*opcode & 0x7F) == 0x00)
            break;
    }
    
    readChar(&c);
    len_msg = c & 0x7f;
    is_masked = c & 0x80;
    if (len_msg == 126) {
        readChar(&c);
        len_msg = c << 8;
        readChar(&c);
        len_msg += c;
    } else if (len_msg == 127) {
        len_msg = 0;
        for (int i = 0; i < 8; i++) {
            readChar(&c);
            len_msg += (c << (7-i)*8);
        }
    }

    if (len_msg == 0) {
        return 0;
    }
    
    if (is_masked) {
        for (i = 0; i < 4; i++) {
          readChar(&c);
          mask[i] = c;
        }
    }

    int nb = read(message, len_msg, len_msg);
    if (nb != len_msg)
        return false;

    for (i = 0; i < len_msg; i++) {
        message[i] = message[i] ^ mask[i % 4];
    }
    
    *length = len_msg;
    message[len_msg] = '\0';

    return true;
}

bool Websocket::close() {

    int ret = socket.close();
    if (ret < 0) {
        ERR("Could not disconnect");
        return false;
    }
    return true;
}

char* Websocket::getPath() {
    return path;
}

int Websocket::write(char * str, int len) {
    int res = 0, idx = 0;
    
    for (int j = 0; j < MAX_TRY_WRITE; j++) {

        if ((res = socket.send(str + idx, len - idx)) < 0)
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

int Websocket::read(char * str, int len, int min_len) {
    int res = 0, idx = 0;
    
    for (int j = 0; j < MAX_TRY_WRITE; j++) {

        if ((res = socket.recv(str + idx, len - idx)) < 0)
          continue;

        idx += res;
        
        if (idx == len || (min_len != -1 && idx > min_len))
            return idx;
    }
    
    return (idx == 0) ? -1 : idx;
}
