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
    if(constrain(Math_NegativeNum, Math_Zero, Math_PositiveNum) != Math_Zero)
    {
        Serial.println("Error: Math.constrain(): If testVal is less than oneNum, oneNum should be return"); 
    }

    if(constrain(Math_Zero, Math_NegativeNum, Math_PositiveNum) != Math_Zero)
    {
        Serial.println("Error: Math.constrain(): If testVal is between oneNum and secondNum, testVal should be return"); 
    }

    if(constrain(Math_PositiveNum, Math_NegativeNum, Math_Zero) != Math_Zero)
    {
        Serial.println("Error: Math.constrain(): If testVal is greater than secondNum, secondNum should be return"); 
    }  

    delay(LOOP_DELAY);
}
