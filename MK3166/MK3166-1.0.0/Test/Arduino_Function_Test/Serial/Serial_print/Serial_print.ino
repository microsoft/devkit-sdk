int x =0;

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    Serial.println("[Serial]: Test print() and println()")
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
        Serial.print("\t");

        Serial.print(x, DEC);
        Serial.print("\t");

        Serial.print(x, HEX);
        Serial.print("\t");

        Serial.print(x, OCT);
        Serial.print("\t");

        Serial.print(x, BIN);
        Serial.println();
        
        delay(500);
    }

    Serial.println(x, 1);
    Serial.println(12.3456, 2);

    int analogValue = analogRead(ARDUINO_PIN_A2);
    Serial.println(analogValue);
    Serial.println(analogValue, DEC);
    Serial.println(analogValue, HEX);
    Serial.println(analogValue, OCT);
    Serial.println(analogValue, BIN);

    Serial.printf("testing format - String: %s; the length is %d", "hello", 5);

    Serial.println("[Serial]: Done");
    delay(1000);
}
