#define LOOP_DELAY          500

int counter = 1;
int val = 0;

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
    Serial.println("Verify AnalogRead() and AnalogReadResolution()");
    check_analogread();

    Serial.println("Verify AnalogWrite() and AnalogWriteResolution()");
    check_analogwrite();

    delay(LOOP_DELAY);
}

void check_analogread()
{
    analogReadResolution(10);    
    int result  = analogRead(ARDUINO_PIN_A0);
    Serial.printf("ADC 10-bit(default): %d\n", result);
    if(result < 0 || result > 1023)
    {
      Serial.println("Error: For analogReadResolution(10), the value should be between 0 and 1023");
    }
    Serial.println();
    
    analogReadResolution(12);
    result  = analogRead(ARDUINO_PIN_A0);
    Serial.printf("ADC 12-bit: %d\n", result);
    if(result < 0 || result > 4095)
    {
      Serial.println("Error: For analogReadResolution(12), the value should be between 0 and 4095");
    }
    Serial.println();

    analogReadResolution(16);
    result  = analogRead(ARDUINO_PIN_A0);
    Serial.printf("ADC 16-bit: %d\n", result);
    if(result < 0 || result > 65535)
    {
      Serial.println("Error: For analogReadResolution(16), the value should be between 0 and 65535");
    }
    Serial.println();
    
    analogReadResolution(8);
    result  = analogRead(ARDUINO_PIN_A0);
    Serial.printf("ADC 8-bit: %d\n", result);
    if(result < 0 || result > 256)
    {
      Serial.println("Error: For analogReadResolution(8), the value should be between 0 and 256");
    }
    Serial.println();

    analogReadResolution(20);
    result  = analogRead(ARDUINO_PIN_A0);
    Serial.printf("ADC 20-bit: %d\n", result);
    if(result < 0 || result > 1048576)
    {
      Serial.println("Error: For analogReadResolution(20), the value should be between 0 and 1048576");
    }
    Serial.println();
}

void check_analogwrite()
{
    val= analogRead(ARDUINO_PIN_A0);
    Serial.print("Analog Read:");
    Serial.println(val);

    analogWriteResolution(8);
    analogWrite(ARDUINO_PIN_A1, map(val,0,1023, 0, 255));
    Serial.print("8-bit PWM value:");
    Serial.println(map(val,0,1023, 0, 255));

    analogWriteResolution(12);
    analogWrite(ARDUINO_PIN_A1, map(val,0,1023, 0, 4095));
    Serial.print("12-bit PWM value:");
    Serial.println(map(val,0,1023, 0, 4095));

    analogWriteResolution(4);
    analogWrite(ARDUINO_PIN_A1, map(val,0,1023, 0, 15));
    Serial.print("4-bit PWM value:");
    Serial.println(map(val,0,1023, 0, 15));
}
