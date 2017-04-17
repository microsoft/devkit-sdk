int incomingByte = 0;

void setup()
{
    Serial.begin(115200);
    while(!Serial)
    {
        ;
    }
}

void loop()
{
    // if(Serial.available()>0)
    // {

    // }
    Serial.println("[Serial]: Test related functions");
    Serial.println(Serial.available());

    incomingByte = Serial.read();
    Serial.println(incomingByte);

    Serial.end();
    Serial.println("Test serial.end()");
    Serial.begin(9600);
    Serial.println("Test serial.begin()");

    if(!Serial.find("abc"))
    {
      Serial.println("failed to find the string abc");
    }

    if(!Serial.findUntil("abc","#"))
    {
      Serial.println("failed to find the string abc");
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
