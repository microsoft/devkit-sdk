#include <ArduinoUnit.h>
#define LOOP_DELAY          500

volatile byte state=LOW;

void setup()
{
    Serial.println(__FILE__);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(USER_BUTTON_A, INPUT);
    
    // need to be updated 
    // as this function is not declare in this scope
    //attachInterrupt(digitalPinToInterrupt(USER_BUTTON_A),blink,CHANGE);
}

void loop() {
  Test::run();
}

test(check_external_interrupt)
{
    Serial.println("Test attachInterrupt()");
    digitalWrite(LED_BUILTIN, state);

    delay(LOOP_DELAY);
}

void blink()
{
    state != state;
}
