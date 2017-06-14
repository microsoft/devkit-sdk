#define LOOP_DELAY          500

int counter = 1;
unsigned long T_time;

void setup() {
  Serial.println(">> Start");
  Serial.println(__FILE__);

  pinMode(LED_BUILTIN, OUTPUT);
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
    Serial.println("Verify delay() and delayMicroseconds()");
    check_timedelay();

    Serial.println("Verify micros() and millis()");
    check_timemicros_timemillis();
}

void check_timedelay()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(LOOP_DELAY); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(LOOP_DELAY);

    digitalWrite(LED_BUILTIN, HIGH);
    delayMicroseconds(LOOP_DELAY);
    digitalWrite(LED_BUILTIN, LOW);
    delayMicroseconds(LOOP_DELAY);
}

void check_timemicros_timemillis()
{
    Serial.print("Time:");
    T_time = micros();
    
    //print time
    Serial.println(T_time);
    delay(LOOP_DELAY); 

    T_time = millis();
  
    //print time
    Serial.println(T_time);
    delay(LOOP_DELAY);
}
