void setup() {
  Serial.begin(9600);
}

int val = 0;

void loop() {  
  Serial.println("[Analog I/0]: Test analyRead()");
  analogReference(DEFAULT);
  
  val= analogRead(ARDUINO_PIN_A2);

  if(val <0 || val >1023)
  {
    Serial.printf("[Analog I/0]: Error: The return value should be between 0 and 1023, but the actual value is %d", val);
  }
  
  Serial.println("[Analog I/0]: Done");
  delay(1000);
}
