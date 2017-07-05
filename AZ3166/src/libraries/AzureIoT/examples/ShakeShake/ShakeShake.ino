#include "AzureIotHub.h"
#include "OledDisplay.h"
#include "AZ3166WiFi.h"
#include "_iothub_client_sample_mqtt.h"
#include "Sensor.h"

#define RGB_LED_BRIGHTNESS  32
#define LOOP_DELAY          100

#define HEARTBEAT_INTERVAL  120.0
#define PULL_TIMEOUT        15.0

#define RECONNECT_THRESHOLD 3

// 0 - idle
// 1 - shaking
// 2 - do work
// 3 - received
static volatile int status;

DevI2C *ext_i2c;
LSM6DSLSensor *acc_gyro;

static RGB_LED rgbLed;

static char msgText[1024];
static char temp[100];

static char *msgHeader = NULL;
static char *msgBody = NULL;
static int msgStart = 0;

static volatile boolean eventSent = false;

bool hasWifi = false;

static const char* iot_event = "{\"topic\":\"iot\"}";

static time_t time_hb;
static time_t time_sending_timeout;

static int restart_iot_client = 0;

void MessageSendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
  eventSent = true;
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    // If get back the send confirmation msg clear the re-connect count
    restart_iot_client = 0;
  }
  else if (result == IOTHUB_CLIENT_CONFIRMATION_MESSAGE_TIMEOUT)
  {
    // This should be a connection issue, re-connect
    restart_iot_client = RECONNECT_THRESHOLD;
  }
}

static char printable_char(char c)
{
  return c;
  return (c >= 0x20 and c != 0x7f) ? c : '?';  
}

void TwitterMessageCallback(const char *tweet, int lenTweet)
{
  if (status < 2 || lenTweet == NULL)
  {
      return;
  }
  
  if (lenTweet > 220){
      lenTweet = 220;
  }
  
  int i = 0;
  int j = 0;
  // The header
  for (; i < lenTweet; i++)
  {
      if(tweet[i] == '\n')
      {
          msgHeader[j] = 0;
          break;
      }
      msgHeader[j++] = printable_char(tweet[i]);
  }
  // The body
  j = 0;
  for (; i < lenTweet; i++)
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
  restart_iot_client = 0;
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

static void SendEventToIoTHub(const char* msg)
{
  // Check connection first
  if (restart_iot_client >= RECONNECT_THRESHOLD)
  {
    iothub_client_sample_mqtt_close();
    iothub_client_sample_mqtt_init();
    restart_iot_client = 0;
  }
  // Then send
  iothub_client_sample_send_event((const unsigned char *)msg);
}

static void DoHeartBeat(void)
{
  time_t start;
  time_t cur;
  
  time(&start);
  if (difftime(start, time_hb) < HEARTBEAT_INTERVAL)
  {
    return;
  }
  
  eventSent = false;
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(">>Heartbeat<<");
  SendEventToIoTHub(iot_event);
  for (;;)
  {
    iothub_client_sample_mqtt_loop();
    if (eventSent)
    {
      break;
    }
    time(&cur);
    double diff = difftime(cur, start);
    if (diff >= PULL_TIMEOUT)
    {
      // Not get back the send confirmation msg, increase the restart count by 1
      Serial.println(">>Failed to retrieve the sending confirmation message: timeout.");
      restart_iot_client++;
      break;
    }
  }
  time(&time_hb);
  digitalWrite(LED_BUILTIN, LOW);
}

void setup()
{
  msgHeader = (char*) malloc(20);
  msgBody = (char*) malloc(200);
  msgBody[0] = 0;
  status = 0;
  restart_iot_client = 0;
  
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
  
  iothub_client_sample_mqtt_init();
  
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
      Screen.print(1, "  Shake!");
      Screen.print(2, "     Shake!");
      Screen.print(3, "        Shake!");
      status = 1;
      msgBody[0] = 0;
      rgbLed.setColor(0, RGB_LED_BRIGHTNESS, 0);
      acc_gyro->resetStepCounter();
    }
}

static void DoShake()
{
  int steps = 0;
  acc_gyro->getStepCounter(&steps);
  if (steps > 2)
  {
    time(&time_hb); // Reset the heartbeat clock

    // Trigger the twitter
    eventSent = false;
    SendEventToIoTHub(iot_event);
    status = 2;
    time(&time_sending_timeout);  // Start sending timeout clock
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    Screen.print(1, " ");
    Screen.print(2, " Processing...");
    Screen.print(3, " ");
  }
}

static void DoWork()
{
  time(&time_hb); // Reset the heartbeat clock

  iothub_client_sample_mqtt_loop();

  if (status != 3)
  {
    // Not get the tweet, check the sending timeout
    time_t cur;
    time(&cur);
    double diff = difftime(cur, time_sending_timeout);
    if (diff >= PULL_TIMEOUT)
    {
      if (!eventSent)
      {
        // If not get back the send confirmation msg then increase the restart count by 1
        restart_iot_client ++;
      }
      
      // Switch back to status 0
      Screen.print(1, "No tweets...");
      Screen.print(2, "Press A to Shake!");
      Screen.print(3, " ");
      rgbLed.setColor(0, 0, 0);
      status = 0;
    }
  }

  time(&time_hb);
}

static void DoReceived()
{
  time(&time_hb); // Reset the heartbeat clock

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
    
    DoHeartBeat();
  }
    
  delay(LOOP_DELAY);
}
