void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    Serial.println("[Time]: Test delay()");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000); //wait for a second
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);  //wait for a second
    Serial.println("[Time]: Done");
}