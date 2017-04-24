void setup()
{
    Serial.begin(115200);
}

void loop()
{
    Serial.println("[Analog I/0]: Test analyReadResolution()");
    analogReadResolution(10);    
    int result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 10-bit(default): %d", result);
    if(result < 0 || result > 1023)
    {
      Serial.println("[Analog I/0]:Error: For analogReadResolution(10), the value should be between 0 and 1023");
    }
    Serial.println();
    
    analogReadResolution(12);
    result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 12-bit: %d", result);
    if(result < 0 || result > 4095)
    {
      Serial.println("[Analog I/0]:Error: For analogReadResolution(12), the value should be between 0 and 4095");
    }
    Serial.println();

    analogReadResolution(16);
    result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 16-bit: %d", result);
    if(result < 0 || result > 65535)
    {
      Serial.println("[Analog I/0]:Error: For analogReadResolution(16), the value should be between 0 and 65535");
    }
    Serial.println();
    
    analogReadResolution(8);
    result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 8-bit: %d", result);
    if(result < 0 || result > 256)
    {
      Serial.println("[Analog I/0]:Error: For analogReadResolution(8), the value should be between 0 and 256");
    }
    Serial.println();

    analogReadResolution(20);
    result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 20-bit: %d", result);
    if(result < 0 || result > 1048576)
    {
      Serial.println("[Analog I/0]:Error: For analogReadResolution(20), the value should be between 0 and 1048576");
    }
    Serial.println();
    
    Serial.println("[Analog I/0]: Done");
    delay(1000);
}
