void setup(){
    Serial.begin(115200);
}

void loop(){
    Serial.println("[Math]: Test sqrt()");
    if(sqrt(9) != 3)
    {
      Serial.println("[Math]: Error: Math.sqrt(): Failed to get the square root of 9");
    }

    if(sqrt(1.0) != 1)
    {
      Serial.println("[Math]: Error: Math.sqrt(): Failed to get the square root of 1");
    }
    
    Serial.println("[Math]: Done");
    delay(1000);       
}
