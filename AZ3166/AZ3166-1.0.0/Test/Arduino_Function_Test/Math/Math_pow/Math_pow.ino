void setup(){
    Serial.begin(115200);
}

void loop(){
    Serial.println("[Math]: Test pow()");
    if(pow(2,3) != 8)
    {
      Serial.println("[Math]: Error: Math.pow(): Failed to get the result of 2^3");
    }

    if(pow(9,0.5) != 3)
    {
      Serial.println("[Math]: Error: Math.pow(): Failed to get the result of 9^0.5");
    }

    Serial.println("[Math]: Done");  
    delay(1000);       
}
