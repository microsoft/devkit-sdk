int incomingByte=0;
char readVal[3];

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    Serial.println("[Serial]: Test read() and write()");
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

    Serial.println("[Serial]: Done");
    delay(1000);
}
