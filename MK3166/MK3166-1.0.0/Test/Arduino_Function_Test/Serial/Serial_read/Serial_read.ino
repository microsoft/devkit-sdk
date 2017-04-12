int incomingByte=0;
char readVal[3];

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    Serial.println("[Serial]: Test read() and write()")
    incomingByte = Serial.read();
    Serial.println(incomingByte);

    Serial.println("Testing readBytes()...");
    Serial.readBytes(readVal,3);
    Serial.println(readVal);

    Serial.println("Testing readBytesUntil()...");
    char c = '1';
    Serial.readBytesUntil(c, readVal,3);

    Serial.setTimeout(1000);
    Serial.println("Testing readstring()...");
    String str = Serial.readString();
    Serial.println(str);

    Serial.setTimeout(1000);
    Serial.println("Testing readstringUntil()...");
    str = Serial.readStringUntil('#');
    Serial.println(str);

    Serial.println("Testing write()...");
    Serial.println(Serial.write(45));  
    Serial.println(Serial.write("hello"));

    Serial.println("[Serial]: Done");
    delay(1000);
}
