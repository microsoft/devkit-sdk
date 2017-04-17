#include "_iothub_client_sample_mqtt.h"
#include "AzureIotHub.h"
#include "WiFi.h"

static char msgText[1024];
static char temp[100];

#define pulsePin USER_BUTTON_A
#define lightPin LED_BUILTIN
bool buttonStateChanged = false;
volatile int state = HIGH;
bool hasWifi = false;

void pulseStateHook()
{
  int newstate = digitalRead(pulsePin);
  if(newstate != state)
  {
      buttonStateChanged = true;
      state = newstate;
  }
}

void InitWiFi()
{
  Screen.print("Azure IoT DevKit\r\n \r\nConnecting...\r\n");
  
  if(WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running...      \r\n");
  }
  else
  {
     Screen.print(1, "No Wi-Fi\r\n                ");
  }
}

void setup()
{
  InitWiFi();
  if(!hasWifi)
  {
    return;
  }
  
  pinMode(lightPin, OUTPUT);
  pinMode(pulsePin, INPUT);

  Serial.begin(115200);
  Serial.println("start");
  
  state = digitalRead(pulsePin);
  digitalWrite(lightPin, HIGH);

  iothub_client_sample_mqtt_init();
}
void reportPulse()
{
  if (state == LOW)
  {
    digitalWrite(lightPin, LOW);
    Serial.println("*****************************SendEvent********************************");
    sprintf(msgText, "{\"topic\":\"%s\"}", "#iot");
    iothub_client_sample_send_event((const unsigned char *)msgText);
  }
  else
  {
    digitalWrite(lightPin, HIGH);
  }
}
void loop()
{
  if(hasWifi)
  {
    pulseStateHook();
    if (buttonStateChanged)
    {
      buttonStateChanged = false;
      reportPulse();
    }
    iothub_client_sample_mqtt_loop();
  }
  else
  {
    delay(500);
    return;
  }
}
