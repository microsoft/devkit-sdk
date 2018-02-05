#include "ArduinoUnit.h"
#include "HTS221Sensor.h"
#include "lis2mdlSensor.h"
#include "LSM6DSLSensor.h"
#include "RGB_LED.h"
#include "AZ3166WiFi.h"
#include "SystemWiFi.h"
#include "config.h"

DevI2C *i2c;

void setup() {
  i2c = new DevI2C(D14, D15);
}

void loop() {
  Test::run();
}
