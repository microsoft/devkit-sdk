void setup(){
  Serial.begin(9600);
}

void loop(){
    Serial.println("[Bits]: Test Bits related functions");
    int value = 22;

    int x = bitRead(value,3);
    Serial.println(x);
    if(x!=0)
    {
        Serial.println("[Bits]: Error: bitRead(22,3): 22 in binary is 0010110, the 3rd bit is 0");        
    }

    x=bitWrite(value,3,1);
    Serial.println(x);
    if(x!=30)
    {
        Serial.println("[Bits]: Error: bitWrite(22,3,1): it should be 30 after change the 3rd bit to 1");
    }

    x = bitSet(value,3); 
    Serial.println(x);
    if(x!=30)
    {
        Serial.println("[Bits]: Error: bitSet(22,3): it should be 30 after change the 3rd bit to 1");
    }

    x = bitClear(value, 2);
    Serial.println(x);
    if(x!=26)
    {
        Serial.println("[Bits]: Error: bitClear(22,2): it should be 26 after clean the 2nd bit");
    }

    x= bit(2);
    Serial.println(x);
    if(x!=4)
    {
        Serial.println("[Bits]: Error: bit(2): it should be 4");
    }

    Serial.println("[Bits]: Done");
    delay(1000);
}
