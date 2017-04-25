void setup()
{
    Serial.begin(115200);
}

void loop()
{
    Serial.println("[Serial]: Test serialEvent");

    Serial.println("[Serial]: Done");
    delay(1000);
}

void serialEvent()
{
  if(Serial.available())
  {
    Serial.print("call SerialEvent()\n");
  }
}
