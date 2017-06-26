#define LOOP_DELAY          500

int counter = 1;
volatile byte state=LOW;

void setup()
{
    Serial.println(">> Start");
    Serial.println(__FILE__);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(USER_BUTTON_A, INPUT);
    
    // need to be updated 
    // as this function is not declare in this scope
    //attachInterrupt(digitalPinToInterrupt(USER_BUTTON_A),blink,CHANGE);
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
    Serial.println("Test attachInterrupt()");
    digitalWrite(LED_BUILTIN, state);

    delay(LOOP_DELAY);
}

void blink()
{
    state != state;
}
