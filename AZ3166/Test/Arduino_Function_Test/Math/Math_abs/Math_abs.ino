#define LOOP_DELAY          500
#define Math_PositiveNum    10
#define Math_Zero           0
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
    if(abs(Math_PositiveNum) != Math_PositiveNum )
    {
        Serial.println("Error: Math.abs(): If x is greater than 0, the absolute value should be equal to x");
    }

    if(abs(Math_Zero) != Math_Zero )
    {
        Serial.println("Error: Math.abs(): If number is 0, the absolute value should be 0");
    }

    if(abs(Math_NegativeNum) != Math_NegativeNum * (-1) )
    {
        Serial.println("Error: Math.abs(): If x is less than 0, the absolute value should be equal to -x");
    }
    
    delay(LOOP_DELAY);
}
