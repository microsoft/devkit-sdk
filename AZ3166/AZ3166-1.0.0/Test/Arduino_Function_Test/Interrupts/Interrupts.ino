void setup()
{
    Serial.begin(9600);
}

void loop()
{
    Serial.println("[Interrupt]: Test Interrupt()");
    noInterrupts();

    interrupts();

    Serial.println("[Interrupt]: Done");
    delay(1000);
}