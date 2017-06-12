#define LOOP_DELAY          500

int counter = 1;
int val = 0;

void setup() {
  Serial.println(">> Start");
  Serial.println(__FILE__);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(USER_BUTTON_A, INPUT);
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
    Serial.println("You can press button A to check the LED status");
    val = digitalRead(USER_BUTTON_A);
    
    digitalWrite(LED_BUILTIN, val);
    
    Serial.println(val);
    delay(LOOP_DELAY); 
}
