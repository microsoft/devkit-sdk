void setup(){
    Serial.begin(115200);
}

void loop(){
    Serial.println("[Math]: Test constrain()");
    int oneNum=1;
    int secondNum = 100;
    
    int testVal =0;
    if(constrain(testVal, oneNum, secondNum) != oneNum)
    {
        Serial.println("[Math]: Error: Math.constrain(): If testVal is less than oneNum, oneNum should be return"); 
    }

    testVal =50;
    if(constrain(testVal, oneNum, secondNum) != testVal)
    {
        Serial.println("[Math]: Error: Math.constrain(): If testVal is between oneNum and secondNum, testVal should be return"); 
    }

    testVal =200;
    if(constrain(testVal, oneNum, secondNum) != secondNum)
    {
        Serial.println("[Math]: Error: Math.constrain(): If testVal is greater than secondNum, secondNum should be return"); 
    }  

    Serial.println("[Math]: Done");
    delay(1000);
}