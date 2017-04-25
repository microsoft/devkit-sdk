#include "../Test/TestData.h"

void setup(){
    Serial.begin(115200);
}

void loop(){
    Serial.println("[Math]: Test min()");
    int minVal = min(Math_NegativeNum,Math_PositiveNum);

    if(minVal != Math_NegativeNum)
    {        
        Serial.println("[Math]: Error: Math.min(): Failed to get the min number.");
    }    

    Serial.println("[Math]: Done");
    delay(1000);       
}
