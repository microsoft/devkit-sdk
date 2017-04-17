volatile byte state=LOW;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(ARDUINO_PIN_D1, INPUT);
    attachInterrupt(digitalPinToInterrupt(ARDUINO_PIN_D1),blink,CHANGE);
}

void loop()
{
    Serial.println("[External Interrupt]: Test attachInterrupt()");
    digitalWrite(LED_BUILTIN, state);

    Serial.println("[External Interrupt]: Done");
    delay(1000);
}

void blink()
{
    state != state;
}
