#include "_iothub_client_sample_mqtt.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "AzureIotHub.h"

static char msgText[1024];
static char temp[100];


#define pulsePin USER_BUTTON_A
#define pulseLedPin LED_AZURE
#define lightPin LED_USER
bool buttonStateChanged = false;
volatile byte state = HIGH;
void turnLightOn() {
  Serial.println("*****************************TurnLightOn********************************");
  digitalWrite(lightPin, LOW);
}
void pulseStateHook()
{
  auto prev = state;
  state = digitalRead(pulsePin);
  buttonStateChanged = prev != state;
}
void setup()
{
  pinMode(pulseLedPin, OUTPUT);
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
  pulseStateHook();
  if (buttonStateChanged)
  {
    buttonStateChanged = false;
    reportPulse();
  }
  digitalWrite(pulseLedPin, state);
  iothub_client_sample_mqtt_loop();
}
