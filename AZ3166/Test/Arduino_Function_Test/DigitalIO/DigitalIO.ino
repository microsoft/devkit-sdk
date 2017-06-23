#include <ArduinoUnit.h>
#define LOOP_DELAY          500

int val = 0;

void setup() {
  Serial.println(__FILE__);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(USER_BUTTON_A, INPUT);
}

void loop() {
  Test::run();
}

test(check_digital_io)
{
    Serial.println("You can press button A to check the LED status");
    val = digitalRead(USER_BUTTON_A);
    assertEqual(val, HIGH);
    
    digitalWrite(LED_BUILTIN, val);
    assertEqual(digitalRead(LED_BUILTIN), val);    
    delay(LOOP_DELAY); 

    digitalWrite(LED_BUILTIN, HIGH);
    assertEqual(digitalRead(LED_BUILTIN), HIGH);
    delay(LOOP_DELAY);
    
    digitalWrite(LED_BUILTIN, LOW);
    assertEqual(digitalRead(LED_BUILTIN), LOW);
    delay(LOOP_DELAY);
}
