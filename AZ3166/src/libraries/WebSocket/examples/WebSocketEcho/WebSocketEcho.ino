#include "AZ3166WiFi.h"
#include "WebSocketClient.h"

static bool hasWifi;
static bool isWsConnected;

static char webSocketServerUrl[] = "ws://echo.websocket.org/"; // or use ws://demos.kaazing.com/echo
static WebSocketClient *wsClient;
char wsBuffer[1024];
char wifiBuff[128];
int msgCount;

void initWiFi()
{
  Screen.print("WiFi \r\n \r\nConnecting...\r\n             \r\n");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    snprintf(wifiBuff, 128, "WiFi Connected\r\n%s\r\n%s\r\n \r\n", WiFi.SSID(), ip.get_address());
    Screen.print(wifiBuff);

    hasWifi = true;
  }
  else
  {
    snprintf(wifiBuff, 128, "No WiFi\r\nEnter AP Mode\r\nto config\r\n                 \r\n");
    Screen.print(wifiBuff);
  }
}

bool connectWebSocket()
{
  Screen.clean();
  Screen.print(0, "Connect to WS...");

  if (wsClient == NULL)
  {
    wsClient = new WebSocketClient(webSocketServerUrl);
  }

  isWsConnected = wsClient->connect();
  if (isWsConnected)
  {
    Screen.print(1, "connect WS successfully.");
    Serial.println("WebSocket connect successfully.");
  }
  else
  {
    Screen.print(1, "Connect WS failed.");
    Serial.print("WebSocket connection failed, isWsConnected: ");
    Serial.println(isWsConnected);
  }

  return isWsConnected;
}

void setup()
{
  hasWifi = false;
  isWsConnected = false;
  msgCount = 0;

  initWiFi();
  if (hasWifi)
  {
    connectWebSocket();
  }

  pinMode(USER_BUTTON_A, INPUT);
}

void loop()
{
  if (hasWifi)
  {
    if (!isWsConnected)
    {
      Screen.clean();
      Screen.print(0, "DevKit WebSocket");
      Screen.print(1, "Press button A to connect WS.", true);

      while (digitalRead(USER_BUTTON_A) != LOW)
      {
        delay(10);
      }

      connectWebSocket();
    }

    if (isWsConnected)
    {
      int len = 0;
      char msgBuffer[256];

      for (int i = 0; i < 3; i++)
      {
        // Send message to WebSocket server
        sprintf(msgBuffer, "Hello WebSocket %d", msgCount);
        int res = wsClient->send(msgBuffer, strlen(msgBuffer));
        if (res > 0)
        {
          Screen.clean();
          Screen.print(0, "WS send:");
          Screen.print(1, msgBuffer, true);
          Serial.printf("Send message %d successfully.\r\n", msgCount);
          msgCount++;
        }

        // Receive message from WebSocket Server
        bool isEndOfMessage = false;
        WebSocketReceiveResult *recvResult = NULL;
        Screen.clean();
        Screen.print(0, "WS receive:");
        Screen.print(1, "receiving...");
        Serial.print("WS receive:");

        do
        {
          recvResult = wsClient->receive(wsBuffer, sizeof(wsBuffer));

          if (recvResult && recvResult->length > 0)
          {
            len = recvResult->length;
            isEndOfMessage = recvResult->isEndOfMessage;

            Screen.print(1, wsBuffer, true);
            Serial.println(wsBuffer);

            memset(wsBuffer, 0, sizeof(wsBuffer));
            delay(100);
          }
        } while (!isEndOfMessage);
      }

      // Close WebSocket after sending/receiving messages
      if (wsClient->close())
      {
        isWsConnected = false;
        Screen.print(1, "Close WS successfully.", true);
        Serial.println("Close WS successfully.");
      }
      else
      {
        Screen.print(1, "Close WS failed.", true);
        Serial.println("Close WS failed.");
      }
    }
  }

  delay(1000);
}
