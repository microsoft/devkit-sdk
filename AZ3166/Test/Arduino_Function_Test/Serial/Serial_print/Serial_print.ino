#define LOOP_DELAY          500

int counter = 1;
int x =0;

void setup()
{
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
    Serial.print("No Format");
    Serial.print("\t");

    Serial.print("DEC");
    Serial.print("\t");

    Serial.print("HEX");
    Serial.print("\t");

    Serial.print("OCT");
    Serial.print("\t");

    Serial.print("BIN");
    Serial.print("\n");

    for(x=0; x<20; x++)
    {
        Serial.print(x);
        Serial.print("\t\t");

        Serial.print(x, DEC);
        Serial.print("\t");

        Serial.print(x, HEX);
        Serial.print("\t");

        Serial.print(x, OCT);
        Serial.print("\t");

        Serial.print(x, BIN);
        Serial.println();
        
        delay(LOOP_DELAY);
    }

    Serial.println(x, 1);
    Serial.println(12.3456, 2);

    int analogValue = analogRead(ARDUINO_PIN_A2);
    Serial.println(analogValue);
    Serial.println(analogValue, DEC);
    Serial.println(analogValue, HEX);
    Serial.println(analogValue, OCT);
    Serial.println(analogValue, BIN);

    // test printf() and printf_P()
    Serial.printf("testing printf() - String: %s; the length is %d\n", "hello", 5);
    Serial.printf_P("testing printf_P() - String: %s; the length is %d", "hello", 5);
    Serial.println();

    // test print(double n, int digits)
    Serial.print(11.2233,2);
    Serial.println();
    
    delay(LOOP_DELAY);
}
