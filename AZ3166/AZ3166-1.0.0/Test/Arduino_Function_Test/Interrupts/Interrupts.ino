void setup()
{
    Serial.begin(115200);
}

void loop()
{
    Serial.println("[Interrupt]: Test Interrupt()");
    noInterrupts();

    interrupts();

    Serial.println("[Interrupt]: Done");
    delay(1000);
}