#define LOOP_DELAY          500

int counter = 1;

void setup(){
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
      Serial.println();
      Serial.println(">> End");
    }
    
    counter++;
  }
}

void runCase(){
    if(sqrt(9) != 3)
    {
      Serial.println("Error: Math.sqrt(): Failed to get the square root of 9");
    }

    if(sqrt(1.0) != 1)
    {
      Serial.println("Error: Math.sqrt(): Failed to get the square root of 1");
    }
    
    delay(LOOP_DELAY);       
}
