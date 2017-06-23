#include "ArduinoUnit.h"

#define LOOP_DELAY          500

int val = 0;

void setup() {
  Serial.println(__FILE__);
}

void loop() {  
  Test::run();
}

test(check_analogread)
{
    analogReadResolution(10);    
    int result  = analogRead(ARDUINO_PIN_A0);
    assertMoreOrEqual(result, 0);
    assertLessOrEqual(result, 1023);
    
    analogReadResolution(12);
    result  = analogRead(ARDUINO_PIN_A0);
    assertMoreOrEqual(result, 0);
    assertLessOrEqual(result, 4095);
}

test(check_analogwrite)
{
    analogReadResolution(10);
    val= analogRead(ARDUINO_PIN_A0);
    Serial.print("Analog Read:");
    Serial.println(val);

    analogWriteResolution(10);
    analogWrite(ARDUINO_PIN_A1, val);
    assertEqual(analogRead(ARDUINO_PIN_A1), val);
}