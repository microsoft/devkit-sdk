volatile byte state=LOW;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(GPIO_PIN_DAC, INPUT);
    attachInterrupt(digitalPinToInterrupt(GPIO_PIN_DAC),blink,CHANGE);
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