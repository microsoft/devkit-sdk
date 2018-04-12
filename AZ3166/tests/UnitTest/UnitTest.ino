#include "ArduinoUnit.h"
#include "HTS221Sensor.h"
#include "lis2mdlSensor.h"
#include "LSM6DSLSensor.h"
#include "RGB_LED.h"
#include "AZ3166WiFi.h"
#include "SystemWiFi.h"
#include "config.h"

void setup() {
  pinInit();
  I2CInit();
}

void loop() {
  Test::run();
}
