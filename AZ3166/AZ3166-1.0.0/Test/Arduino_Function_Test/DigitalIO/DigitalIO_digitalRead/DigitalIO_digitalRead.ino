void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(USER_BUTTON_A, INPUT);
}

int val=0;

void loop() { 
    Serial.println("[Digital I/O]: Test digitalRead() and digitalWrite()");
    Serial.println("You can press button A to check the LED status");
    val = digitalRead(USER_BUTTON_A);
    
    digitalWrite(LED_BUILTIN, val);
    
    Serial.println(val);
    Serial.println("[Digital I/O]: Done");
    delay(1000);  //wait for a second
}
