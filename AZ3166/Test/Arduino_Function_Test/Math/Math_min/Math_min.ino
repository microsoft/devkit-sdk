#define LOOP_DELAY          500
#define Math_PositiveNum    10
#define Math_NegativeNum    -1

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
    int minVal = min(Math_NegativeNum,Math_PositiveNum);

    if(minVal != Math_NegativeNum)
    {        
        Serial.println("Error: Math.min(): Failed to get the min number.");
    }    

    delay(LOOP_DELAY);       
}
