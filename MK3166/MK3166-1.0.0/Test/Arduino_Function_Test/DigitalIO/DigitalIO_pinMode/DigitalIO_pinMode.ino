void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    Serial.println("[Digital I/O]: Test pinMode()");    
    
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000); //wait for a second
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);  //wait for a second

    Serial.println("[Digital I/O]: Done");
}