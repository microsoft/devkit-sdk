#include "LSM6DSLSensor.h"
#include "AzureIotHub.h"
#include "OLEDDisplay.h"
#include "RGB_LED.h"
#include "AZ3166WiFi.h"
#include "_iothub_client_sample_mqtt.h"

#define RGB_LED_BRIGHTNESS  16
#define LOOP_DELAY          50
#define HEARTBEAT_INTERVAL  (60000 / LOOP_DELAY)

// 0 - idle
// 1 - shaking
// 2 - sending
// 3 - recieved
static int status;

static boolean hasWifi;
static boolean eventSent;

static DevI2C ext_i2c(D14, D15);
static LSM6DSLSensor acc_gyro(ext_i2c, D4, D5);

static RGB_LED rgbLed;

static char msgHeader[20];
static char msgBody[200];
static int msgStart;

static int heartbeat;

void _SendConfirmationCallback(void)
{
    eventSent = true;
}

void _showMessage(const char *tweet, int lenTweet)
{
  if (status > 1 && tweet != NULL)
  {
    // Copy the body of the tweet, ignore the first line
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
        msgHeader[j++] = tweet[i];
    }
    j = 0;
    for (; i < lenTweet; i++)
    {
      if (tweet[i] != '\r' && tweet[i] != '\n')
      {
        msgBody[j++] = tweet[i];
      }
    }
    msgBody[j] = 0;
    Serial.println(msgBody);
    msgStart = 0;  // Enable scrolling
    
    status = 3;
  }
}

void SendEventToIoTHub()
{
  // Here the DeviceID is hardcoded, it shall be the same one in the IoT Hub connection string otherwise no tweets will come back
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
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Initialize the motion sensor
  Screen.print(3, " > Motion sensor     ");
  acc_gyro.init(NULL);
  acc_gyro.enable_x();
  acc_gyro.enable_g();
  acc_gyro.enable_pedometer();
  acc_gyro.set_pedometer_threshold(LSM6DSL_PEDOMETER_THRESHOLD_MID_LOW);
}

static void DoHeartBeat(void)
{
  if (heartbeat >= HEARTBEAT_INTERVAL)
  {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println(">>Heartbeat<<");
    eventSent = false;
    iothub_client_sample_send_event((const unsigned char *)"{\"topic\":\"iot\", \"DeviceID\":\"Heartbeat\", \"event\":\"heartbeat\"}");
    for (int i =0; i < 20; i++)
    {
      iothub_client_sample_mqtt_loop();
      if (eventSent)
      {
        break;
      }
    }
    if (!eventSent)
    {
      Serial.println("Failed to get response from IoT hub: timeout.");
    }
    heartbeat = 0;
    digitalWrite(LED_BUILTIN, HIGH);
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
  
  // Run one heartbeat to warm up the Azure service
  rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
  heartbeat = HEARTBEAT_INTERVAL;
  DoHeartBeat();
  rgbLed.setColor(0, 0, 0);
  
  Screen.print(2, "Press A to Shake!    ");
  Screen.print(3, "                     ");
}

static void DoIdle(void)
{
    if (digitalRead(USER_BUTTON_A) == LOW)
    {
      acc_gyro.reset_step_counter();
      Screen.print(0, "Azure IoT DevKit     ");
      Screen.print(1, "  Shake!             ");
      Screen.print(2, "     Shake!          ");
      Screen.print(3, "        Shake!       ");
      status = 1;
      msgStart = -1;
      rgbLed.setColor(0, RGB_LED_BRIGHTNESS, 0);
      acc_gyro.reset_step_counter();
    }
    heartbeat++;
}

static void DoShake(void)
{
  uint16_t steps = 0;
  acc_gyro.get_step_counter(&steps);
  if (steps > 2)
  {
    // Trigger the twitter
    SendEventToIoTHub();
    status = 2;
    heartbeat = 0;
    rgbLed.setColor(RGB_LED_BRIGHTNESS, 0, 0);
    Screen.print(1, "                     ");
    Screen.print(2, " Processing...       ");
    Screen.print(3, "                     ");
  }
  else
  {
    heartbeat++;
  }
}

static void DoSending(void)
{
  iothub_client_sample_mqtt_loop();
  heartbeat = 0;
}

static void DoRecived(void)
{
  // Show the tweet
  Screen.clean();
  Screen.print(0, msgHeader);
  Screen.print(1, msgBody, true);
  rgbLed.setColor(0, 0, RGB_LED_BRIGHTNESS);
  status = 0;
}

static void ScrollTweet(void)
{
  if (msgStart >=0 && digitalRead(USER_BUTTON_B) == LOW)
  {
    // Scroll it if Button B has been pressed
    msgStart += 16;
    if (msgStart >= strlen(msgBody))
    {
        msgStart = 0;
    }
        
    // Clean the msg screen
    Screen.print(1, "                     ");
    Screen.print(2, "                     ");
    Screen.print(3, "                     ");
    // Update it
    Screen.print(1, &msgBody[msgStart], true);
  }
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
      DoSending();
      break;
      
    case 3:
      DoRecived();
      break;
  }
  
  ScrollTweet();
  
  DoHeartBeat();
  
  delay(LOOP_DELAY);
}
