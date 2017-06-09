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
    if(pow(2,3) != 8)
    {
      Serial.println("Error: Math.pow(): Failed to get the result of 2^3");
    }

    if(pow(9,0.5) != 3)
    {
      Serial.println("Error: Math.pow(): Failed to get the result of 9^0.5");
    }

    delay(LOOP_DELAY);       
}
