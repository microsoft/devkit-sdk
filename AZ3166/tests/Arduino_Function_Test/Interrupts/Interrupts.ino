#include <ArduinoUnit.h>
#define LOOP_DELAY          500

void setup()
{
    Serial.println(__FILE__);
}

void loop() {
  Test::run(); // need to update this case
}

test(interrupt)
{
    noInterrupts();

    interrupts();

    delay(LOOP_DELAY);
}