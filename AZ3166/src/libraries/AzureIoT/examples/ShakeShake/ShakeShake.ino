// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "IoTHubMQTTClient.h"
#include "OledDisplay.h"
#include "Sensor.h"
#include "ShakeUI.h"
#include "SystemTickCounter.h"

#define RGB_LED_BRIGHTNESS  32
#define LOOP_DELAY          100

#define HEARTBEAT_INTERVAL  300000
#define PULL_TIMEOUT        15000

#define MSG_HEADER_SIZE     20
#define MSG_BODY_SIZE       200

#define SCROOL_OFFSET       16

// 0 - idle
// 1 - shaking
// 2 - do work
// 3 - received
static volatile int status;

DevI2C *ext_i2c;
LSM6DSLSensor *acc_gyro;

static RGB_LED rgbLed;

static char msgHeader[MSG_HEADER_SIZE];
static char msgBody[MSG_BODY_SIZE];
static int msgStart = 0;

bool hasWifi = false;

static const char* iot_event = "{\"topic\":\"iot\"}";
static const char* iot_event_heartbeat = "{\"topic\":\"\"}";

static uint64_t hb_interval_ms;
static uint64_t sending_timeout_ms;

static int shake_progress;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static char printable_char(char c)
{
  return (c >= 0x20 and c != 0x7f) ? c : '?';  
}

static void ScrollTweet()
{
  if (msgBody[0] != 0 && digitalRead(USER_BUTTON_B) == LOW)
  {
    // Scroll it if Button B has been pressed
    if (msgStart < 0)
    {
      // First time
      Screen.print(0, msgHeader);
      msgStart = 0;
    }
    else
    {
      msgStart += SCROOL_OFFSET;
      if (msgStart >= strlen(msgBody))
      {
          msgStart = 0;
      }
    }
          
    // Clean the msg screen
    Screen.print(1, " ");
    Screen.print(2, " ");
    Screen.print(3, " ");
    // Update it
    Screen.print(1, &msgBody[msgStart], true);
  }
}

static void InitWiFi()
{
  Screen.clean();
  Screen.print(0, "IoT DevKit");
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
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

static void ShowProgress()
{
  if (shake_progress == 1)
  {
    Screen.print(0, "Processing...");
    Screen.print(1, "   DevKit");
    Screen.print(2, "   Azure");
    Screen.print(3, "   Twitter");
  }

  DrawCheckBox(1, 0, (shake_progress >= 1) ? 1 : 0);
  DrawCheckBox(2, 0, (shake_progress >= 2) ? 1 : 0);
  DrawCheckBox(3, 0, (shake_progress == 4) ? 1 : 0);
  
  delay(500);
}

static void LogShakeResult(const char* result)
{
  char sz[32];
  sprintf(sz, "progress-%d", shake_progress);
  LogTrace(result, sz);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback functions
void TwitterMessageCallback(const char *tweet, int lenTweet)
{
  if (status != 2 || tweet == NULL)
  {
      return;
  }

  shake_progress = 3;

  if (lenTweet == 0)
  {
    msgHeader[0] = 0;
    msgBody[0] = 0;
  }
  else
  {
    // Split into header and body
    int i = 0;
    int j = 0;
    // The header
    for (; i < min(lenTweet, sizeof(msgHeader)); i++)
    {
        if(tweet[i] == '\n')
        {
            break;
        }
        msgHeader[j++] = printable_char(tweet[i]);
    }
    msgHeader[j] = 0;
    Serial.println(msgHeader);
    if (strcmp(msgHeader, "No new tweet.") == 0)
    {
      // There is no new tweet from Twitter.
      msgHeader[0] = 0;
      msgBody[0] = 0;
    }
    else
    {
      // The body
      j = 0;
      for (; i < min(lenTweet, sizeof(msgHeader) + sizeof(msgBody)); i++)
      {
        if (tweet[i] != '\r' && tweet[i] != '\n')
        {
          msgBody[j++] = printable_char(tweet[i]);
        }
      }
      msgBody[j] = 0;
      Serial.println(msgBody);

      shake_progress = 4;
    }
  }
  
  ShowProgress();
  status = 3;
  msgStart = -SCROOL_OFFSET;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actions
static void DoHeartBeat()
{
  if ((int)(SystemTickCounterRead() - hb_interval_ms) < HEARTBEAT_INTERVAL)
  {
    return;
  }

  Serial.println(">>Heartbeat<<");

  DigitalOut LedUser(LED_BUILTIN);
  LedUser = 1;
  
  IoTHubMQTT_SendEvent(iot_event_heartbeat);
  
  LedUser = 0;
  
  // Reset
  hb_interval_ms = SystemTickCounterRead();
}

static void DoIdle()
{
    if (digitalRead(USER_BUTTON_A) == LOW)
    {
      // Enter Shake mode
      status = 1;
      msgHeader[0] = 0;
      msgBody[0] = 0;
      shake_progress = 0;

      rgbLed.setColor(0, RGB_LED_BRIGHTNESS, 0);

      Screen.clean();
      Screen.print(0, "   Shake Shake!");
      DrawSmallTweetIcon(0, 0);
      DrawShakeAnimation();

      acc_gyro->resetStepCounter();
    }
    IoTHubMQTT_Check();
}

static void NoTweets()
{
  Screen.clean();
  Screen.print(0, "No tweets...");
  Screen.print(3, "Press A to Shake!");
  DrawTweetImage(1, 20, 0);
  rgbLed.setColor(0, 0, 0);

  // Log shake failed message
  LogShakeResult("ShakeShakeFailed");
  
  // Switch back to status 0
  status = 0;
}

static void DoShake()
{
  int steps = 0;
  acc_gyro->getStepCounter(&steps);
  if (steps > 2)
  {
    status = 2;
    shake_progress = 1;

    // LED
    DigitalOut LedUser(LED_BUILTIN);
    LedUser = 1;
    // RGB LED
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    // Update the screen
    ShowProgress();
    
    // Send event to IoT hub
    if (IoTHubMQTT_SendEvent(iot_event))
    {
      // Waiting for the tweet
      shake_progress = 2;
      ShowProgress();
      
      // Start retrieving tweet timeout clock
      sending_timeout_ms = SystemTickCounterRead();
    }
    else
    {
      NoTweets();
    }
    LedUser = 0;
  }
  else
  {
    DrawShakeAnimation();
  }
}

static void DoWork()
{
  IoTHubMQTT_Check();

  if (status != 3)
  {
    // Not get the tweet, check the sending timeout
    if ((int)(SystemTickCounterRead() - sending_timeout_ms) >= PULL_TIMEOUT)
    {
      NoTweets();
    }
  }
}

static void DoReceived()
{
  Screen.clean();
  if (shake_progress == 4)
  {
    Screen.print(0, "New tweet!");
    Screen.print(3, "Press B to read!");

    DrawTweetImage(1, 20, 1);

    // Log shake succeed message
    LogShakeResult("ShakeShakeSucceed");
  }
  else
  {
    NoTweets();
  }
  rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
  status = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  msgHeader[0] = 0;
  msgBody[0] = 0;
  status = 0;
  shake_progress = 0;
  
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
  
  LogShakeResult("ShakeShakeSetup");
    
  // Initialize LEDs
  Screen.print(3, " > LEDs");
  rgbLed.turnOff();
  
  // Initialize button
  Screen.print(3, " > Button");
  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize the motion sensor
  Screen.print(3, " > Motion sensor     ");
  ext_i2c = new DevI2C(D14, D15);
  acc_gyro = new LSM6DSLSensor(*ext_i2c, D4, D5);
  acc_gyro->init(NULL); 
  acc_gyro->enableAccelerator();
  acc_gyro->enableGyroscope();
  acc_gyro->enablePedometer();
  acc_gyro->setPedometerThreshold(LSM6DSL_PEDOMETER_THRESHOLD_MID_LOW);

  Screen.print(3, " > IoT Hub");
  
  IoTHubMQTT_Init();
  IoTHubMQTT_SetMessageCallback(TwitterMessageCallback);
  
  rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
  hb_interval_ms = -(HEARTBEAT_INTERVAL);   // Trigger heart beat immediately
  DoHeartBeat();
  rgbLed.setColor(0, 0, 0);
  
  Screen.print(2, "Press A to Shake!");
  Screen.print(3, " ");
}

void loop()
{
  if (hasWifi)
  {
    switch(status)
    {
      case 0:
        DoIdle();
        break;
      
      case 1:
        DoShake();
        break;
        
      case 2:
        DoWork();
        break;
        
      case 3:
        DoReceived();
        break;
    }
    
    ScrollTweet();
    
    if (status == 0)
    {
      DoHeartBeat();
    }
    else
    {
      hb_interval_ms = SystemTickCounterRead();
    }
  }

  delay(LOOP_DELAY);
}
