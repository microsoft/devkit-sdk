// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/shake-shake/?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
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

#define SCROLL_OFFSET       16

static const char* iot_event = "{\"topic\":\"iot\"}"; // The #hashtag is 'iot' here, you can change to other keyword you want
static const char* iot_event_heartbeat = "{\"topic\":\"\"}";  // Empty for heart beat

// Application running status
// 0 - idle
// 1 - shaking
// 2 - do work
static int app_status;

// Peripherals 
static DevI2C *ext_i2c;
static LSM6DSLSensor *acc_gyro;
static RGB_LED rgbLed;

// Tweet message
static char msgHeader[MSG_HEADER_SIZE];
static char msgBody[MSG_BODY_SIZE];
static int msgStart = 0;

// Indicate whether WiFi is ready
static bool hasWifi = false;

// Time interval check for heart beat
static uint64_t hb_interval_ms;
// Time interval check for retrieving the tweet
static uint64_t tweet_timeout_ms;

// Shake shake processing status
// 0 - Not start
// 1 - Shaked and sending message to IoT hub
// 2 - Message has been sent to IoT hub (get confirmed)
// 3 - Got the tweet message, sometime it's empty
// 4 - The tweet is OK and show it on the screen
static int shake_progress;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static char PrintableChar(char c)
{
  return (c >= 0x20 and c != 0x7f) ? c : '?';  
}

static void ScrollTweet()
{
  if (msgBody[0] == 0)
  {
    return;
  }
  
  if (msgStart < 0)
  {
    // First time
    DrawAppTitle(msgHeader);
    msgStart = 0;
  }
  else
  {
    msgStart += SCROLL_OFFSET;
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

static void InitWiFi()
{
  Screen.clean();
  DrawAppTitle("IoT DevKit");
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

static void HeartBeat()
{
  if ((int)(SystemTickCounterRead() - hb_interval_ms) < HEARTBEAT_INTERVAL)
  {
    return;
  }

  Serial.println(">>Heartbeat<<");

  DigitalOut LedUser(LED_BUILTIN);
  LedUser = 1;
  // Send heart beat message
  IoTHubMQTT_SendEvent(iot_event_heartbeat);
  LedUser = 0;
  
  // Reset
  hb_interval_ms = SystemTickCounterRead();
}

static bool ParseTweet(const char *tweet, int lenTweet)
{
  if (lenTweet == 0)
  {
    msgHeader[0] = 0;
    msgBody[0] = 0;
    return false;
  }
  
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
      msgHeader[j++] = PrintableChar(tweet[i]);
  }
  msgHeader[j] = 0;
  Serial.println(msgHeader);
  if (strcmp(msgHeader, "No new tweet.") == 0)
  {
    // Not a tweet from Twitter, there must be something wrong on the Service side.
    msgHeader[0] = 0;
    msgBody[0] = 0;
    return false;
  }
  else
  {
    // The body
    j = 0;
    for (; i < min(lenTweet, sizeof(msgHeader) + sizeof(msgBody)); i++)
    {
      if (tweet[i] != '\r' && tweet[i] != '\n')
      {
        msgBody[j++] = PrintableChar(tweet[i]);
      }
    }
    msgBody[j] = 0;
    Serial.println(msgBody);
    return true;
  }
}

static void ShowShakeProgress()
{
  if (shake_progress == 1)
  {
    DrawAppTitle("Processing...");
    Screen.print(1, "   DevKit");
    Screen.print(2, "   Azure");
    Screen.print(3, "   Twitter");
  }

  DrawCheckBox(1, 0, (shake_progress >= 1) ? 1 : 0);
  DrawCheckBox(2, 0, (shake_progress >= 2) ? 1 : 0);
  DrawCheckBox(3, 0, (shake_progress == 4) ? 1 : 0);
  
  delay(500);
}

static void NoTweets()
{
  Screen.clean();
  if (WiFi.status() == WL_CONNECTED)
  {
    DrawAppTitle("No tweets...");
  }
  else
  {
    DrawAppTitle("No Wi-Fi...");
  }
  Screen.print(3, "Press A to Shake!");
  DrawTweetImage(1, 20, 0);

  // Turn off the RGB LED
  rgbLed.setColor(0, 0, 0);

  // Log the shake failed message
  LogShakeResult("ShakeShakeFailed");
  
  // Switch back to idle mode
  app_status = 0;
}

static void LogShakeResult(const char* result)
{
  char sz[32];
  sprintf(sz, "progress-%d", shake_progress);
  LogTrace(result, sz);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callback functions
static void TwitterMessageCallback(const char *tweet, int lenTweet)
{
  if (app_status != 2 || tweet == NULL)
  {
    // Return if not under do work mode or the tweet is empty.
    return;
  }

  if (ParseTweet(tweet, lenTweet))
  {
    // The tweet message is OK
    shake_progress = 4;
  }
  else
  {
    // Got message from Azure, but not a tweet from Twitter.
    // There must be something wrong on the Service side.
    shake_progress = 3;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actions
static void DoIdle()
{
    if (digitalRead(USER_BUTTON_A) == LOW)
    {
      // Enter Shake mode
      app_status = 1;
      
      msgHeader[0] = 0;
      msgBody[0] = 0;
      shake_progress = 0;

      // Set the RGB LED to green
      rgbLed.setColor(0, RGB_LED_BRIGHTNESS, 0);

      Screen.clean();
      DrawAppTitle("Shake Shake!");

      acc_gyro->resetStepCounter();
    }
    else if (digitalRead(USER_BUTTON_B) == LOW)
    {
      // Show / scroll the tweet
      ScrollTweet();
    }
    
    // Heart beat
    HeartBeat();
    
    // Check with the IoT hub
    IoTHubMQTT_Check();
}

static void DoShake()
{
  int steps = 0;
  acc_gyro->getStepCounter(&steps);
  if (steps > 2)
  {
    // Enter the do work mode
    app_status = 2;
    // Shake detected
    shake_progress = 1;

    // LED
    DigitalOut LedUser(LED_BUILTIN);
    LedUser = 1;
    // Set RGB LED to red
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    // Update the screen
    ShowShakeProgress();
    // Send to IoT hub
    if (IoTHubMQTT_SendEvent(iot_event))
    {
      if (shake_progress < 2)
      {
        // Because the tweet may return quickly and the TwitterMessageCallback be executed before run to here,
        // So check the shake_progress to avoid set the wrong value.
        // IoT hub has got the message
        shake_progress = 2;
      }
      // Update the screen
      ShowShakeProgress();
      // Start retrieving tweet timeout clock
      tweet_timeout_ms = SystemTickCounterRead();
    }
    else
    {
      // Failed to send message to IoT hub
      NoTweets();
    }
    LedUser = 0;
  }
  else
  {
    // Draw the animation
    DrawShakeAnimation();
  }
}

static void DoWork()
{
  if ((int)(SystemTickCounterRead() - tweet_timeout_ms) >= PULL_TIMEOUT)
  {
    // Timeout
    NoTweets();
  }
  // Check with the IoT hub
  IoTHubMQTT_Check();

  if (shake_progress > 2)
  {
    // Got the tweet message
    // Update the progress
    ShowShakeProgress();

    if (shake_progress == 4)
    {
      Screen.clean();
      // Got the tweet
      // Show the action UI and let user to choose read or shake again
      DrawAppTitle("New tweet!");
      Screen.print(3, "Press B to read!");
      DrawTweetImage(1, 20, 1);

      // Prepare for reading and scrolling
      msgStart = -SCROLL_OFFSET;

      // Set RGB LED to blue, means for reading
      rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
      // Switch back to idle mode
      app_status = 0;

      // Log shake succeed message
      LogShakeResult("ShakeShakeSucceed");
    }
    else
    {
      // No tweet
      NoTweets();
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  msgHeader[0] = 0;
  msgBody[0] = 0;
  app_status = 0;
  shake_progress = 0;
  
  Screen.init();
  DrawAppTitle("IoT DevKit");
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
  HeartBeat();
  rgbLed.setColor(0, 0, 0);
  
  Screen.print(2, "Press A to Shake!");
  Screen.print(3, " ");
}

void loop()
{
  if (hasWifi)
  {
    switch(app_status)
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
    }
  }

  delay(LOOP_DELAY);
}
