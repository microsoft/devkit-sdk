#include "../Test/TestData.h"

void setup(){
    Serial.begin(115200);
}

void loop(){
    Serial.println("[Math]: Test max()");
    int maxVal = max(Math_NegativeNum,Math_PositiveNum);

    if(maxVal != Math_PositiveNum)
    {
        Serial.println("[Math]: Error: Math.max(): Failed to get the max number.");
    }    

    Serial.println("[Math]: Done");
    delay(1000);       
}