int incomingByte = 0;

void setup()
{
    Serial.begin(115200);    
}

void loop()
{
    Serial.println("[Serial]: Test related functions");
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
      Serial.println("[Serial]: Error: failed to find the string test");
    }

    if(!Serial.findUntil("test","#"))
    {
      Serial.println("[Serial]: Error: failed to find the string test");
    }

    Serial.flush();

    //long result = Serial.parseInt();
    //Serial.println(result);

    //float resultFloat = Serial.parseFloat();
    //Serial.println(resultFloat);

    Serial.println(Serial.peek());    

    Serial.println("[Serial]: Done");
    delay(1000);
}
