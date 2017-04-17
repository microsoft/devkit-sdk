void setup(){
    Serial.begin(9600);
}

void loop(){
    Serial.println("[Math]: Test min()");
    int oneNum=10;
    int secondNum = 20;
    int minVal = min(oneNum,secondNum);

    if(minVal != oneNum)
    {        
        Serial.println("[Math]: Error: Math.min(): Failed to get the min number.");
    }    

    Serial.println("[Math]: Done");
    delay(1000);       
}
