#include "_iothub_client_sample_mqtt.h"
#include "AzureIotHub.h"
#include "WiFi.h"

static char msgText[1024];
static char temp[100];

#define pulsePin USER_BUTTON_A
#define lightPin LED_USER
bool buttonStateChanged = false;
volatile byte state = HIGH;
bool hasWifi = false;

void turnLightOn() 
{
  Serial.println("*****************************TurnLightOn********************************");
  digitalWrite(lightPin, LOW);
}

void pulseStateHook()
{
  auto prev = state;
  state = digitalRead(pulsePin);
  buttonStateChanged = prev != state;
}

void InitWiFi()
{
  Screen.print("Azure IoT DevKit\r\n \r\nConnecting...\r\n");
  
  if(WiFi.begin() == WL_CONNECTED)
  {
    Screen.print(1, WiFiInterface()->get_ip_address());
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
  digitalWrite(lightPin, HIGH);

  iothub_client_sample_mqtt_init();
}
void reportPulse()
{
  if (state == LOW)
  {
    // turn off LED_AZURE first
    digitalWrite(lightPin, HIGH);
    Serial.println("*****************************SendEvent********************************");
    sprintf(msgText, "{\"topic\":\"%s\"}", "#iot");
    iothub_client_sample_send_event((const unsigned char *)msgText);
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
