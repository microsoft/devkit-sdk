#define LOOP_DELAY          500

int counter = 1;
unsigned long T_time;

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
      Serial.println();
      Serial.println(">> End");
    }
    
    counter++;
  }
}

void runCase()
{
  Serial.print("Time:");
  T_time = micros();
  
  //print time
  Serial.println(T_time);
  delay(LOOP_DELAY); 
}