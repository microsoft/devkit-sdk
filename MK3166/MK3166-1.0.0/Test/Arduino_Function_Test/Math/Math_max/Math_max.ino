void setup(){
    
}

void loop(){
    Serial.println("[Math]: Test max()");
    int oneNum=10;
    int secondNum = 20;
    int maxVal = max(oneNum,secondNum);

    if(maxVal != secondNum)
    {
        Serial.println("[Math]: Error: Math.max(): Failed to get the max number.");
    }    

    Serial.println("[Math]: Done");
    delay(1000);       
}