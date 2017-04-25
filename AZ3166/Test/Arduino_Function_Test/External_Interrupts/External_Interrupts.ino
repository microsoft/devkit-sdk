volatile byte state=LOW;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(USER_BUTTON_A, INPUT);
    
    // need to be updated 
    // as this function is not declare in this scope
    //attachInterrupt(digitalPinToInterrupt(USER_BUTTON_A),blink,CHANGE);
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
