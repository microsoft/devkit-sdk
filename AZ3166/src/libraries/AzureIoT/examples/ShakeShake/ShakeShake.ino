#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "IoTHubMQTTClient.h"
#include "OledDisplay.h"
#include "Sensor.h"

#define RGB_LED_BRIGHTNESS  32
#define LOOP_DELAY          100

#define HEARTBEAT_INTERVAL  300.0
#define PULL_TIMEOUT        15.0

#define MSG_HEADER_SIZE     20
#define MSG_BODY_SIZE       200

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

static time_t time_hb;
static time_t time_sending_timeout;

static int animation = 0;
const char* s1  = "  Shake!";
const char* s2  = "     Shake!";
const char* s3  = "        Shake!";
const char* list[3] = { s1, s2, s3 };

static void ShakeAnimation()
{
  Screen.print(1, list[(animation / 200 + 0) % 3]);
  Screen.print(2, list[(animation / 200 + 1) % 3]);
  Screen.print(3, list[(animation / 200 + 2) % 3]);
  animation += LOOP_DELAY;
}

static char printable_char(char c)
{
  return (c >= 0x20 and c != 0x7f) ? c : '?';  
}

void TwitterMessageCallback(const char *tweet, int lenTweet)
{
  if (status < 2 || lenTweet == NULL)
  {
      return;
  }
  
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
  
  Serial.println(msgHeader);
  Serial.println(msgBody);
  
  status = 3;
  msgStart = 0;
  
  LogTrace("ShakeShakeSucceed", NULL);
}

static void ScrollTweet(void)
{
  if (msgBody[0] != 0 && digitalRead(USER_BUTTON_B) == LOW)
  {
    // Scroll it if Button B has been pressed
    msgStart += 16;
    if (msgStart >= strlen(msgBody))
    {
        msgStart = 0;
    }
        
    // Clean the msg screen
    Screen.print(1, " ");
    Screen.print(2, " ");
    Screen.print(3, " ");
    // Update it
    Screen.print(1, &msgBody[msgStart], true);
  }
}

void InitWiFi()
{
  Screen.print("IoT DevKit\r\n \r\nConnecting...\r\n");

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

static void DoHeartBeat(void)
{
  time_t cur;
  
  time(&cur);
  if (difftime(cur, time_hb) < HEARTBEAT_INTERVAL)
  {
    return;
  }

  Serial.println(">>Heartbeat<<");

  DigitalOut LedUser(LED_BUILTIN);
  LedUser = 1;
  
  IoTHubMQTT_SendEvent(iot_event_heartbeat);
  
  LedUser = 0;
  time(&time_hb); //  Reset the clock
}

void setup()
{
  msgHeader[0] = 0;
  msgBody[0] = 0;
  status = 0;
  
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
  
  LogTrace("ShakeShakeSetup", NULL);
  
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
  time_hb = 0;
  DoHeartBeat();
  rgbLed.setColor(0, 0, 0);
  
  Screen.print(2, "Press A to Shake!");
  Screen.print(3, " ");
}

static void DoIdle()
{
    if (digitalRead(USER_BUTTON_A) == LOW)
    {
      Screen.print(0, "IoT DevKit");
      animation = 0;
      ShakeAnimation();
      status = 1;
      msgBody[0] = 0;
      rgbLed.setColor(0, RGB_LED_BRIGHTNESS, 0);
      acc_gyro->resetStepCounter();
    }
    IoTHubMQTT_Check();
}

static void NoTweets()
{
  Screen.print(1, "No tweets...");
  Screen.print(2, "Press A to Shake!");
  Screen.print(3, " ");
  rgbLed.setColor(0, 0, 0);

  // Switch back to status 0
  status = 0;
}

static void DoShake()
{
  int steps = 0;
  acc_gyro->getStepCounter(&steps);
  if (steps > 2)
  {
    // LED
    DigitalOut LedUser(LED_BUILTIN);
    LedUser = 1;
    // RGB LED
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    // Update the screen
    Screen.print(1, " ");
    Screen.print(2, " Processing...");
    Screen.print(3, " ");
    ThreadAPI_Sleep(10);
    status = 2;
    if (IoTHubMQTT_SendEvent(iot_event))
    {
      // Waiting for the tweet
      Screen.print(2, " Retrieving...");
      
      time(&time_sending_timeout);  // Start retrieving tweet timeout clock
    }
    else
    {
      NoTweets();
      LogTrace("ShakeShakeFailed", "IoT Hub");
    }
    LedUser = 0;
  }
  else
  {
    ShakeAnimation();
  }
}

static void DoWork()
{
  IoTHubMQTT_Check();

  if (status != 3)
  {
    // Not get the tweet, check the sending timeout
    time_t cur;
    time(&cur);
    double diff = difftime(cur, time_sending_timeout);
    
    if (diff >= PULL_TIMEOUT)
    {
      NoTweets();
      LogTrace("ShakeShakeFailed", "Function App");
    }
  }
}

static void DoReceived()
{
  Screen.clean();
  Screen.print(0, msgHeader);
  Screen.print(1, msgBody, true);
  
  rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
  status = 0;
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
      time(&time_hb); //  Reset the clock
    }
  }

  delay(LOOP_DELAY);
}
