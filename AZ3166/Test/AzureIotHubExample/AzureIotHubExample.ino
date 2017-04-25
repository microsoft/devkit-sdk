#include "azure_iot_hub.h"
#include "AZ3166WiFi.h"
#include "iot_hub_mqtt_example.h"

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

  iot_hub_mqtt_example_init();
}
void reportPulse()
{
  if (state == LOW)
  {
    digitalWrite(lightPin, LOW);
    Serial.println("*****************************SendEvent********************************");
    sprintf(msgText, "{\"topic\":\"%s\"}", "#iot");
    iot_hub_mqtt_example_send_event((const unsigned char *)msgText);
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
    iot_hub_mqtt_example_loop();
  }
  else
  {
    delay(500);
    return;
  }
}
