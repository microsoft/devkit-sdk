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
  val= analogRead(ARDUINO_PIN_A2);

  if(val <0 || val >1023)
  {
    Serial.printf("[Analog I/0]: Error: The return value should be between 0 and 1023, but the actual value is %d", val);
  }
  
  delay(LOOP_DELAY);
}
