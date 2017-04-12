void setup(){
    
}

void loop(){
    Serial.println("[Math]: Test abs()");
    int oneNum=1;
    int secondNum = -1;
    
    if(abs(oneNum) != oneNum )
    {
        Serial.println("[Math]: Error: Math.abs(): If x is greater than 0, the absolute value should be equal to x");
    }

    if(abs(0) != 0 )
    {
        Serial.println("[Math]: Error: Math.abs(): If number is 0, the absolute value should be 0");
    }

    if(abs(secondNum) != secondNum * (-1) )
    {
        Serial.println("[Math]: Error: Math.abs(): If x is less than 0, the absolute value should be equal to -x");
    }
    
    Serial.println("[Math]: Done");
    delay(1000);
}
