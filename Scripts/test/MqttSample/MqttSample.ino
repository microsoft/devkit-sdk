#include "iothub_client_sample_mqtt.h"
NetworkInterface* network;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("start");
  iothub_client_sample_mqtt_run();
}

void loop(){
  Serial.println("loop");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
