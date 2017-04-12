void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    Serial.println("[Time]: Test delayMicroseconds()");
    digitalWrite(LED_BUILTIN, HIGH);
    delayMicroseconds(50); //wait for 50ms
    digitalWrite(LED_BUILTIN, LOW);
    delayMicroseconds(50);  //wait for 50ms

    Serial.print("[Time]: Done");
}
