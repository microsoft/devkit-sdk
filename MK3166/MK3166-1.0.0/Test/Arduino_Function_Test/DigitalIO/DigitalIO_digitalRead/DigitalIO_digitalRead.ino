void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ARDUINO_PIN_D1, INPUT);
}

int val=0;

void loop() { 
    Serial.println("[Digital I/O]: Test digitalRead() and digitalWrite()");
    val = digitalRead(ARDUINO_PIN_D1);
    
    digitalWrite(LED_BUILTIN, val);
    
    Serial.println(val);
    Serial.println("[Digital I/O]: Done");
    delay(1000);  //wait for a second
}
