#define LOOP_DELAY          500

int counter = 1;

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
    digitalWrite(LED_BUILTIN, HIGH);
    delay(LOOP_DELAY); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(LOOP_DELAY);  
}