#define LOOP_DELAY          500

int counter = 1;

void setup() {
  Serial.println(">> Start");
  Serial.println(__FILE__);
}

void loop() {  
  while(counter <= 5)
  {
    Serial.printf(">> Start (%d)\r\n", counter);
    runCase();
    Serial.printf(">> End (%d)\r\n", counter); 

    if(counter == 5)
    {
        Serial.println(">> End");
    }
    
    counter++;
  }
}

void runCase()
{
    analogReadResolution(10);    
    int result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 10-bit(default): %d", result);
    if(result < 0 || result > 1023)
    {
      Serial.println("Error: For analogReadResolution(10), the value should be between 0 and 1023");
    }
    Serial.println();
    
    analogReadResolution(12);
    result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 12-bit: %d", result);
    if(result < 0 || result > 4095)
    {
      Serial.println("Error: For analogReadResolution(12), the value should be between 0 and 4095");
    }
    Serial.println();

    analogReadResolution(16);
    result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 16-bit: %d", result);
    if(result < 0 || result > 65535)
    {
      Serial.println("Error: For analogReadResolution(16), the value should be between 0 and 65535");
    }
    Serial.println();
    
    analogReadResolution(8);
    result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 8-bit: %d", result);
    if(result < 0 || result > 256)
    {
      Serial.println("Error: For analogReadResolution(8), the value should be between 0 and 256");
    }
    Serial.println();

    analogReadResolution(20);
    result  = analogRead(ARDUINO_PIN_A2);
    Serial.printf("ADC 20-bit: %d", result);
    if(result < 0 || result > 1048576)
    {
      Serial.println("Error: For analogReadResolution(20), the value should be between 0 and 1048576");
    }
    Serial.println();
    
    delay(LOOP_DELAY);
}
