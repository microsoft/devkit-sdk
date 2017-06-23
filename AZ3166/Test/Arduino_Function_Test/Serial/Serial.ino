#include <ArduinoUnit.h>
#define LOOP_DELAY          500

int incomingByte = 0;
int x =0;
char readVal[3];

void setup()
{
    Serial.println(__FILE__);    
}

void loop() {
    Test::run();
}

test(check_serialprint)
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

    int analogValue = analogRead(ARDUINO_PIN_A0);
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

//due to need to input some string to the serial port manually, skipped it
void check_serialread()
{
    Serial.println("Testing read(), please enter some char");
    incomingByte = Serial.read();
    Serial.println(incomingByte);

    Serial.println("Testing readBytes(), please enter some chars");
    Serial.readBytes(readVal,3);
    Serial.println(readVal);

    Serial.println("Testing readBytesUntil(), please enter some chars until #");
    char untilChar = '#';
    Serial.readBytesUntil(untilChar, readVal,3);

    Serial.setTimeout(2000);
    Serial.println("Testing readstring(), please enter some string");
    String str = Serial.readString();
    Serial.println(str);

    Serial.setTimeout(2000);
    Serial.println("Testing readstringUntil(), please enter some string until #");
    str = Serial.readStringUntil(untilChar);
    Serial.println(str);

    Serial.println("Testing write()...");
    Serial.println(Serial.write(45));  
    Serial.println(Serial.write("hello"));

    delay(LOOP_DELAY);
}

//due to need to input some string to the serial port manually, skipped it
void check_serialothers()
{
    Serial.println(Serial.available());

    incomingByte = Serial.read();
    Serial.println(incomingByte);

    Serial.end();
    Serial.println("Test serial.end()");
    Serial.begin(115200);
    Serial.println("Test serial.begin()");

    Serial.println("Please enter some string which contain 'test'");
    if(!Serial.find("test"))
    {
      Serial.println("Error: failed to find the string test");
    }

    if(!Serial.findUntil("test","#"))
    {
      Serial.println("Error: failed to find the string test");
    }

    Serial.flush();

    //long result = Serial.parseInt();
    //Serial.println(result);

    //float resultFloat = Serial.parseFloat();
    //Serial.println(resultFloat);

    Serial.println(Serial.peek());    

    delay(LOOP_DELAY);
}
