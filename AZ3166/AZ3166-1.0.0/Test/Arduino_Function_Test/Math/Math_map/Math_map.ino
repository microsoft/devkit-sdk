#include "../Test/TestData.h"

void setup(){
    Serial.begin(115200);
}

void loop(){
    Serial.println("[Math]: Test map()");
    int val = analogRead(0);
    val = map(val,0,1023,0,255);

    Serial.println(val);
    if(val < 0 || val > 255)
    {
      Serial.println("[Math]: Error: map(val,0,1023,0,255): The value shoud be between 0 and 255");
    }

    Serial.println("[Math]: Done");
    delay(1000);
}
