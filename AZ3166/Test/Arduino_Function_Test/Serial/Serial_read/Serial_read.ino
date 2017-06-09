#define LOOP_DELAY          500

int counter = 1;
int incomingByte=0;
char readVal[3];

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
