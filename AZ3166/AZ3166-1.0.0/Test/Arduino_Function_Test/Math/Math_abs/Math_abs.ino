#include "../Test/TestData.h"

void setup(){
    Serial.begin(115200);
}

void loop(){
    Serial.println("[Math]: Test abs()");
    
    if(abs(Math_PositiveNum) != Math_PositiveNum )
    {
        Serial.println("[Math]: Error: Math.abs(): If x is greater than 0, the absolute value should be equal to x");
    }

    if(abs(Math_Zero) != Math_Zero )
    {
        Serial.println("[Math]: Error: Math.abs(): If number is 0, the absolute value should be 0");
    }

    if(abs(Math_NegativeNum) != Math_NegativeNum * (-1) )
    {
        Serial.println("[Math]: Error: Math.abs(): If x is less than 0, the absolute value should be equal to -x");
    }
    
    Serial.println("[Math]: Done");
    delay(1000);
}
