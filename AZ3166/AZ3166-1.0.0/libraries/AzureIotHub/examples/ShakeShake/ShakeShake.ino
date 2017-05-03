#include "LSM6DSLSensor.h"
#include "AzureIotHub.h"
#include "OLEDDisplay.h"
#include "RGB_LED.h"
#include "AZ3166WiFi.h"
#include "_iothub_client_sample_mqtt.h"

#define RGB_LED_BRIGHTNESS  16
#define LOOP_DELAY          100

#define HEARTBEAT_INTERVAL  300.0
#define PULL_TIMEOUT        120.0

// 0 - idle
// 1 - shaking
// 2 - sending
// 3 - recieved
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
static time_t time_sending;

void _SendConfirmationCallback(void)
{
  eventSent = true;
}

static char printable_char(char c)
{
  return c;
  return (c >= 0x20 and c != 0x7f) ? c : '?';  
}

void _showMessage(const char *tweet, int lenTweet)
{
  if (status < 2 || lenTweet == NULL)
  {
      return;
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
  // The boday
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
  Screen.print("Azure IoT DevKit\r\n \r\nConnecting...\r\n");

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
  
  eventSent = false;
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(">>Heartbeat<<");
  iothub_client_sample_send_event((const unsigned char *)iot_event);
  for (;;)
  {
    iothub_client_sample_mqtt_loop();
    if (eventSent)
    {
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
  
  Screen.init();
  Screen.print(0, "Azure IoT DevKit");
  Screen.print(2, "Initializing...");
  
  Screen.print(3, " > Serial");
  Serial.begin(115200);
  
  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWiFi();
  
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
  acc_gyro->enable_x();
  acc_gyro->enable_g();
  acc_gyro->enable_pedometer();
  acc_gyro->set_pedometer_threshold(LSM6DSL_PEDOMETER_THRESHOLD_MID_LOW);
    
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
      Screen.print(0, "Azure IoT DevKit");
      Screen.print(1, "  Shake!");
      Screen.print(2, "     Shake!");
      Screen.print(3, "        Shake!");
      status = 1;
      msgBody[0] = 0;
      rgbLed.setColor(0, RGB_LED_BRIGHTNESS, 0);
      acc_gyro->reset_step_counter();
    }
}

static void DoShake()
{
  uint16_t steps = 0;
  acc_gyro->get_step_counter(&steps);
  if (steps > 2)
  {
    // Trigger the twitter
    iothub_client_sample_send_event((const unsigned char *)iot_event);
    status = 2;
    time(&time_sending);
    time(&time_hb);
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    Screen.print(1, " ");
    Screen.print(2, " Processing...");
    Screen.print(3, " ");
  }
}

static void DoWork()
{
  iothub_client_sample_mqtt_loop();
  time_t cur;
  time(&cur);
  if (difftime(cur, time_sending) >= PULL_TIMEOUT)
  {
    // Switch back to status 0
    Screen.print(1, "Ooooops");
    Screen.print(2, " > TIMEOUT");
    Screen.print(3, "Press A to Shake!");
    rgbLed.setColor(0, 0, 0);
    status = 0;
  }

  time(&time_hb);
}

static void DoRecieved()
{
  Screen.clean();
  Screen.print(0, msgHeader);
  Screen.print(1, msgBody, true);
  
  rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
  status = 0;
  
  time(&time_hb);
}
      
void loop()
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
      DoRecieved();
      break;
  }
  
  ScrollTweet();
  
  DoHeartBeat();
  
  delay(LOOP_DELAY);
}
