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
    int val = analogRead(0);
    val = map(val,0,1023,0,255);

    Serial.println(val);
    if(val < 0 || val > 255)
    {
      Serial.println("Error: map(val,0,1023,0,255): The value shoud be between 0 and 255");
    }

    delay(LOOP_DELAY);
}
