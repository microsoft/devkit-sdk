#define LOOP_DELAY          500

int counter = 1;
int val = 0;

void setup() 
{    
  Serial.println(">> Start");
  Serial.println(__FILE__);

  Serial.begin(115200);
  pinMode(ARDUINO_PIN_A3, OUTPUT);
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
  Serial.print("Analog Read:");
  Serial.println(val);

  analogWriteResolution(8);
  analogWrite(ARDUINO_PIN_A3, map(val,0,1023, 0, 255));
  Serial.print("8-bit PWM value:");
  Serial.println(map(val,0,1023, 0, 255));

  analogWriteResolution(12);
  analogWrite(ARDUINO_PIN_A3, map(val,0,1023, 0, 4095));
  Serial.print("12-bit PWM value:");
  Serial.println(map(val,0,1023, 0, 4095));

  analogWriteResolution(4);
  analogWrite(ARDUINO_PIN_A3, map(val,0,1023, 0, 15));
  Serial.print("4-bit PWM value:");
  Serial.println(map(val,0,1023, 0, 15));

  delay(LOOP_DELAY);
}
