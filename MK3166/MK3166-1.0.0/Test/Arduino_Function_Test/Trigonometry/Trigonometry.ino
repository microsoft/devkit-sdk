void setup() {
  
}

void loop() {  
    Serial.println("[Trigonometry]: Test sin(), cos() and tan()");
    double result = sin(0);
    if(result != 0)
    {
        Serial.println("[Trigonometry]: Error: trigonometry.sin(): for sin(0) should be 0");
    }

    result = sin(1.57);
    if(result <-1 || result > 1)
    {
        Serial.println("[Trigonometry]: Error: trigonometry.sin(): the value should be between 0 and 1");
    }

    result = cos(0.45);
    if(result <-1 || result > 1)
    {
        Serial.println("[Trigonometry]: Error: trigonometry.cos(): the value should be between 0 and 1");
    }

    result = tan(3.14);
    Serial.println(result);  
    
    Serial.println("[Trigonometry]: Done");
    delay(1000);
}
