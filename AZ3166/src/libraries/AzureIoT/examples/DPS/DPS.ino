// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "Arduino.h"
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "IoTHubMQTTClient.h"
#include "DPSClient.h"
#include "Websocket.h"
#include "RingBuffer.h"
#include "AudioClassV2.h"

#include "config.h"
#include "utility.h"
#include "SystemTickCounter.h"
#include "SystemVariables.h"

#define HEARTBEAT_INTERVAL  60000
#define RING_BUFFER_SIZE 32000
#define PLAY_DELAY_RATE 0.1

static bool hasWifi = false;
static bool connect_state;
static int lastButtonAState;
static int buttonAState;
static int lastButtonBState;
static int buttonBState;
static volatile int status;
static uint64_t hb_interval_ms;

static AudioClass& Audio = AudioClass::getInstance();

RingBuffer ringBuffer(RING_BUFFER_SIZE);
char readBuffer[AUDIO_CHUNK_SIZE];
char websocketBuffer[5000];

static char emptyAudio[AUDIO_CHUNK_SIZE];
Websocket *websocket;
bool startPlay = false;

// If you have multiple devices(clients) using the same WebSocket server,
// Please ensure the nickName for each client is unique
char * nickName = "devkit-test";
const char * LUISWebAppUrl;

extern char* Global_Device_Endpoint;
extern char* ID_Scope;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void InitWiFi()
{
  Screen.print(2, "Connecting...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
  LogInfo("Enter DeviceTwinCallback");
  char *temp = (char *)malloc(size + 1);
  if (temp == NULL)
  {
    return;
  }
  memcpy(temp, payLoad, size);
  temp[size] = '\0';
  LUISWebAppUrl = parseTwinMessage(updateState, temp);
  if (LUISWebAppUrl == NULL)
  {
    LogError("Failed to retrieve LUIS URL from Device Twin");
  }

  free(temp);
}

void enterIdleState()
{
    status = 0;
    Screen.clean();
    Screen.print(0, "DevKit-luis.ai");
    Screen.print(1, "Press A to start\r\nconversation");
}

void enterActiveState()
{
    status = 1;
    Screen.print(0, "Active");
    Screen.print(1, "> Hold B to talk");
    Screen.print(2, "> Press A to end  conversation", true);
    Serial.println("Hold B to talk or press A to end conversation");
}

void enterRecordingState()
{
    status = 2;
    Screen.clean();
    Screen.print(0,"Recording...");
    Screen.print(1, "Release B to send    ");
    Serial.println("Release B to send    ");
}

void enterServerProcessingState()
{
    status = 3;
    Screen.clean();
    Screen.print(0,"Processing...");
    Screen.print(1, "Thinking...", true);
}

void enterReceivingState()
{
    status = 4;
    Screen.clean();
    Screen.print(0, "Processing...");
    Screen.print(1, "Receiving...");
}

void enterPlayingState()
{
    status = 5;
    Screen.print(0, "Processing...");
    Screen.print(1, "Playing...");
}

int connectWebSocket()
{
    Screen.clean();
    Screen.print(0, "Connect to WS...");
    char *url = getWebSocketUrl();
    websocket = new Websocket(url);
    connect_state = (*websocket).connect();
    if (connect_state == 1)
    {
        Serial.println("WebSocket connect succeeded.");

        // Trigger heart beat immediately
        hb_interval_ms = -(HEARTBEAT_INTERVAL);
        sendHeartbeat();
        return 0;
    }
    else
    {
        Screen.print(1, "Connect WS fail");
        Screen.print(2, "Press A to retry");
        Serial.print("WebSocket connection failed, connect_state: ");
        Serial.println(connect_state);
        return -1;
    }
}

void sendHeartbeat()
{
    if ((int)(SystemTickCounterRead() - hb_interval_ms) < HEARTBEAT_INTERVAL)
    {
      return;
    }

    // Send haertbeart message
    Serial.println(">>Heartbeat<<");      
    int ret = (*websocket).send("heartbeat", 9);
    if (ret ==  -1)
    {
        // Heartbeat failure, disconnet from WS
        enterIdleState();
    }
    // Reset heartbeat
    hb_interval_ms = SystemTickCounterRead();
}

void record()
{
    ringBuffer.clear();
    Audio.format(8000, 16);
    Audio.attachPlay(NULL);
    Audio.attachRecord(recordCallback);
    Audio.startRecord();
}

void play()
{
    Serial.println("start playing");
    enterPlayingState();
    
    Audio.attachRecord(NULL);
    Audio.attachPlay(playCallback);
    Audio.format(8000, 16);
    Audio.startPlay();
    startPlay = true;
}

void stop()
{
    Audio.stop();
    Audio.attachRecord(NULL);
    Audio.attachPlay(NULL);
    startPlay= false;
}

void playCallback(void)
{
    if (ringBuffer.use() < AUDIO_CHUNK_SIZE)
    {
      Audio.write(emptyAudio, AUDIO_CHUNK_SIZE);
      return;
    }
    ringBuffer.get((uint8_t*)readBuffer, AUDIO_CHUNK_SIZE);
    Audio.write(readBuffer, AUDIO_CHUNK_SIZE);
}

void recordCallback(void)
{
    Audio.read(readBuffer, AUDIO_CHUNK_SIZE);
    ringBuffer.put((uint8_t*)readBuffer, AUDIO_CHUNK_SIZE);
}

void setResponseBodyCallback(const char* data, size_t dataSize)
{
    if (status == 3)
    {
        enterReceivingState();
    }
    
    while(ringBuffer.available() < dataSize)
    {
        delay(10);
    }
    
    ringBuffer.put((uint8_t*)data, dataSize);
    if (ringBuffer.use() > RING_BUFFER_SIZE * PLAY_DELAY_RATE && startPlay == false)
    {
        play();
    }
}

char* getWebSocketUrl()
{
    char *url;
    url = (char *)malloc(300);

    // "ws://demobotapp-sandbox.azurewebsites.net/chat?nickName="
    // LUISWebAppUrl = Endpoint=http://demobotapp-wus.azurewebsites.net;clientName=az-04786300cd50;accessToken=YXotMDQ3ODYzMDBjZDUwZHBzLWFpLWRlbW8=
    snprintf(url, 300, "%s%s", LUISWebAppUrl, nickName);
    Serial.print("WebSocket url: ");
    Serial.println(url);
    return url;
}

void doWork()
{
    switch (status)
    {
        // Idle
        case 0:
            buttonAState = digitalRead(USER_BUTTON_A);
            if (buttonAState == LOW)
            {
                if (connectWebSocket() == 0)
                {
                    enterActiveState();
                }
            }
          break;

        // Active state, ready for conversation
        case 1:
            buttonBState = digitalRead(USER_BUTTON_B);
            if (buttonBState == LOW)
            {
                (*websocket).send("pcmstart", 4, 0x02);
                record();
                enterRecordingState();
            }

            buttonAState = digitalRead(USER_BUTTON_A);
            if (buttonAState == LOW)
            {
                if((*websocket).close())
                {
                    Serial.println("WebSocket close succeeded.");
                    Screen.print("End conversation");
                    delay(200);
                    connect_state = 0;
                    enterIdleState();
                }
                else
                {
                    Serial.println("WebSocket close failed.");
                }
            }

            sendHeartbeat();            
            break;
            
        // Recording state
        case 2:
            while(digitalRead(USER_BUTTON_B) == LOW || ringBuffer.use() > 0)
            {
                if (digitalRead(USER_BUTTON_B) == HIGH)
                {
                    stop();
                }
                
                int sz = ringBuffer.get((uint8_t*)websocketBuffer, AUDIO_CHUNK_SIZE);
                if (sz > 0)
                {
                  (*websocket).send(websocketBuffer, sz, 0x00);
                }
            }
    
            if (Audio.getAudioState() == AUDIO_STATE_RECORDING)
            {
                stop();
            }
            
            // Mark binary message end
            (*websocket).send("pcmend", 4, 0x80);
            Serial.println("Your voice message is sent.");
            enterServerProcessingState();
          break;

        // Receiving and playing
        case 3:           
            unsigned char opcode = 0;
            int len = 0;
            bool firstReceived = true;
            while ((opcode & 0x80) == 0x00)
            {
                int receivedSize = (*websocket).read(websocketBuffer, &len, &opcode, firstReceived);
                //printf("receivedSize %d recv len %d opcode %d\r\n", receivedSize, len, opcode);
                
                if (receivedSize == 0){
                    break;
                }
                firstReceived = false;
                setResponseBodyCallback(websocketBuffer, len);
            }
        
            if (startPlay == false)
            {
              play();
            }
            
            while(ringBuffer.use() >= AUDIO_CHUNK_SIZE)
            {
                delay(100);
            }
            stop();
            enterActiveState();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup() {
  // put your setup code here, to run once:

  Screen.init();
  Screen.print(0, "IoT DevKit");
  Screen.print(2, "Initializing...");

  Screen.print(3, " > Serial");
  Serial.begin(115200);
  
  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWiFi();
  if (!hasWifi)
  {
    return;
  }

  const char* registrationId = GetBoardID();
  char iot_message[35] = {0};
  snprintf(iot_message, 34, "{\"deviceId\":\"%s\"}", registrationId);

  Screen.print(3, " > DPS");
  if(DPSClientStart(Global_Device_Endpoint, ID_Scope, registrationId))
  {
    Screen.print(2, "DPS connected!\r\n");
  }
  else
  {
    Screen.print(2, "DPS Failed!\r\n");
    return;
  }

  Screen.print(3, " > IoT Hub(DPS)");
  IoTHubMQTT_Init(true);
  IoTHubMQTT_SetDeviceTwinCallback(DeviceTwinCallback);

  if(!IoTHubMQTT_SendEvent((char *)iot_message)){
      return;
  }

  Screen.print(3, " > LUIS.ai");
  pinMode(USER_BUTTON_A, INPUT);
  lastButtonAState = digitalRead(USER_BUTTON_A);
  pinMode(USER_BUTTON_B, INPUT);
  lastButtonBState = digitalRead(USER_BUTTON_B);

  memset(emptyAudio, 0x0, AUDIO_CHUNK_SIZE);
  enterIdleState();
}

void loop() 
{
    if (LUISWebAppUrl == NULL)
    {
        IoTHubMQTT_Check();
    }
    
    if (hasWifi)
    {
        doWork();
    }

    delay(100);
}
