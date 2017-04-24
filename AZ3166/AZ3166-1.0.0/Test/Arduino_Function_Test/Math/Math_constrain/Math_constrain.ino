#include "../Test/TestData.h"

void setup(){
    Serial.begin(115200);
}

void loop(){
    Serial.println("[Math]: Test constrain()");

    if(constrain(Math_NegativeNum, Math_Zero, Math_PositiveNum) != Math_Zero)
    {
        Serial.println("[Math]: Error: Math.constrain(): If testVal is less than oneNum, oneNum should be return"); 
    }

    if(constrain(Math_Zero, Math_NegativeNum, Math_PositiveNum) != Math_Zero)
    {
        Serial.println("[Math]: Error: Math.constrain(): If testVal is between oneNum and secondNum, testVal should be return"); 
    }

    if(constrain(Math_PositiveNum, Math_NegativeNum, Math_Zero) != Math_Zero)
    {
        Serial.println("[Math]: Error: Math.constrain(): If testVal is greater than secondNum, secondNum should be return"); 
    }  

    Serial.println("[Math]: Done");
    delay(1000);
}
