#include <ArduinoUnit.h>
#define LOOP_DELAY          500

long randNum;

void setup(){
    Serial.println(__FILE__); 

    randomSeed(analogRead(ARDUINO_PIN_A0));
}

void loop() {
    Test::run();
}

test(check_random)
{
    randNum = random(300);
    assertLess(randNum, 300);
    assertMoreOrEqual(randNum, 0);

    randNum = random(10,20);
    assertLess(randNum, 20);
    assertMoreOrEqual(randNum, 10);

    delay(LOOP_DELAY);
}
