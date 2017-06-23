#include <ArduinoUnit.h>

#define LOOP_DELAY          500
#define Math_PositiveNum    10
#define Math_Zero           0
#define Math_NegativeNum    -1

void setup(){
    Serial.println(__FILE__);
}

void loop() {
    Test::run();
}

test(check_mathabs)
{
    assertEqual(abs(Math_PositiveNum), Math_PositiveNum);
    assertEqual(abs(Math_Zero), Math_Zero);
    assertEqual(abs(Math_NegativeNum), Math_NegativeNum * (-1));

    delay(LOOP_DELAY);
}

test(check_mathconstrain)
{
    // If testVal is less than oneNum, oneNum should be return
    assertEqual(constrain(Math_NegativeNum, Math_Zero, Math_PositiveNum), Math_Zero);

    // If testVal is between oneNum and secondNum, testVal should be return
    assertEqual(constrain(Math_Zero, Math_NegativeNum, Math_PositiveNum), Math_Zero);

    //If testVal is greater than secondNum, secondNum should be return
    assertEqual(constrain(Math_PositiveNum, Math_NegativeNum, Math_Zero), Math_Zero);

    delay(LOOP_DELAY);
}

test(check_mathmap)
{
    int val = analogRead(ARDUINO_PIN_A0);
    val = map(val,0,1023,0,255);

    // map(val,0,1023,0,255): The value shoud be between 0 and 255
    assertMoreOrEqual(val,0);
    assertLessOrEqual(val,255);

    delay(LOOP_DELAY);
}

test(check_mathmax)
{
    assertEqual(max(Math_NegativeNum,Math_PositiveNum), Math_PositiveNum);
    
    delay(LOOP_DELAY); 
}

test(check_mathmin)
{
    assertEqual(min(Math_NegativeNum,Math_PositiveNum), Math_NegativeNum);

    delay(LOOP_DELAY);
}

test(check_mathpow)
{
    assertEqual(pow(2,3), 8);
    assertEqual(pow(9,0.5), 3);

    delay(LOOP_DELAY); 
}

test(check_mathsqrt)
{
    assertEqual(sqrt(9), 3);
    assertEqual(sqrt(1.0), 1);
    
    delay(LOOP_DELAY);
}
