#define LOOP_DELAY          500

int counter = 1;
int val = 0;

void setup() {
  Serial.println(">> Start");
  Serial.println(__FILE__);

  Serial.begin(115200);
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
        Serial.println(">> End");
    }
    
    counter++;
  }
}

void runCase()
{
  val= analogRead(ARDUINO_PIN_A2);
  analogWrite(LED_BUILTIN, val/4);

  delay(LOOP_DELAY);
}
