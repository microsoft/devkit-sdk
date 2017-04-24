#include "LSM6DSLSensor.h"
#include "AzureIotHub.h"
#include "OLEDDisplay.h"
#include "RGB_LED.h"
#include "AZ3166WiFi.h"
#include "_iothub_client_sample_mqtt.h"

#define RGB_LED_BRIGHTNESS 16

// 0 - idle
// 1 - shaking
// 2 - sending
// 3 - waiting for tweets
static int status;

static boolean hasWifi;
static boolean eventSent;

static DevI2C ext_i2c(D14, D15);
static LSM6DSLSensor acc_gyro(ext_i2c, D4, D5);

static RGB_LED rgbLed;

static char msgBuff[200];
static int msgStart;

void _SendConfirmationCallback(void)
{
    eventSent = true;
}

void _showMessage(const char *tweet, int lenTweet)
{
  if (status > 1 && tweet != NULL)
  {
    Screen.clean();
    Screen.print(tweet, true);
    
    status = 0;
    rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
    
    // Copy the body of the tweet, ignore the first line
    boolean startCopy = false;
    int j = 0;
    for (int i = 0; i < lenTweet; i++)
    {
      if (!startCopy && tweet[i] == '\n')
      {
        startCopy = true;
      }
      else if (startCopy)
      {
        if (tweet[i] != '\r' && tweet[i] != '\n')
        {
          msgBuff[j++] = tweet[i];
        }
      }
    }
    msgBuff[j] = 0;
    Serial.println(msgBuff);
    msgStart = 0;  // Enable scrolling
  }
}

void SendEventToIoTHub()
{
  iothub_client_sample_send_event((const unsigned char *)"{\"topic\":\"iot\", \"DeviceID\":\"myDevice1\"}");
}

static void InitWiFi()
{
  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
  }
  else
  {
    Screen.print(1, "No Wi-Fi           ");
  }
}

static void InitBoard(void)
{
  Screen.clean();
  Screen.print(0, "Azure IoT DevKit     ");
  Screen.print(2, "Initializing...      ");
  
  Screen.print(3, " > Serial            ");
  Serial.begin(115200);
  
  // Initialize the WiFi module
  Screen.print(3, " > WiFi              ");
  hasWifi = false;
  InitWiFi();
  
  // Initialize LEDs
  Screen.print(3, " > LEDs              ");
  rgbLed.turnOff();
  
  // Initialize button
  Screen.print(3, " > Button            ");
  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);
  
  // Initialize the motion sensor
  Screen.print(3, " > Motion sensor     ");
  acc_gyro.init(NULL);
  acc_gyro.enable_x();
  acc_gyro.enable_g();
  acc_gyro.enable_pedometer();
  acc_gyro.set_pedometer_threshold(LSM6DSL_PEDOMETER_THRESHOLD_MID_LOW);
}

static void ScrollTweet(void)
{
  if (msgStart >=0 && digitalRead(USER_BUTTON_B) == LOW)
  {
    // Scroll it if Button B has been pressed
    msgStart += 16;
    if (msgStart >= strlen(msgBuff))
    {
        msgStart = 0;
    }
        
    // Clean the msg screen
    Screen.print(1, "                     ");
    Screen.print(2, "                     ");
    Screen.print(3, "                     ");
    // Update it
    Screen.print(1, &msgBuff[msgStart], true);
  }
}

void setup()
{
  InitBoard();
  if (!hasWifi)
  {
    Screen.print(3, " > Fault         ");
    return;
  }
  
  Screen.print(3, " > IoT Hub           ");
  status = 0;
  msgStart = -1;
  iothub_client_sample_mqtt_init();
  eventSent = false;
  SendEventToIoTHub();
  rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
  while (!eventSent)
  {
    iothub_client_sample_mqtt_loop();
  }
  rgbLed.setColor(0, 0, 0);
  
  Screen.print(2, "Press A to Shake!    ");
  Screen.print(3, "                     ");
}

void loop()
{
  switch(status)
  {
    case 0:
      if (digitalRead(USER_BUTTON_A) == LOW)
      {
        Screen.print(0, "Azure IoT DevKit     ");
        Screen.print(1, "  Shake!             ");
        Screen.print(2, "     Shake!          ");
        Screen.print(3, "        Shake!       ");
        status = 1;
        msgStart = -1;
        rgbLed.setColor(0, RGB_LED_BRIGHTNESS, 0);
        acc_gyro.reset_step_counter();
      }
      break;
    
    case 1:
      {
        uint16_t steps = 0;
        acc_gyro.get_step_counter(&steps);
        if (steps > 2)
        {
          acc_gyro.reset_step_counter();
          // Trigger the twitter
          SendEventToIoTHub();
          status = 2;
          rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
          Screen.print(1, "                     ");
          Screen.print(2, " Processing...       ");
          Screen.print(3, "                     ");
        }
      }
      break;
      
    case 2:
      iothub_client_sample_mqtt_loop();
      break;
  }
  ScrollTweet();
  
  delay(50);
}