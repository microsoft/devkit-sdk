String inputStr="";
boolean strComplete= false;

void setup()
{
    Serial.begin(115200);
    inputStr.reserve(200);
}

void loop()
{
    Serial.println("[Serial]: Test serialEvent");
    if(strComplete)
    {
        Serial.println(inputStr);
        inputStr="";
        strComplete = false;
    }

    Serial.println("[Serial]: Done");
    delay(1000);
}

void serialEvent()
{
    //while(Serial.available())
    //{
        char inChar = (char)Serial.read();
        inputStr += inChar;

        if(inChar == '\n')
        {
            strComplete = true;
        }
    //}
}
