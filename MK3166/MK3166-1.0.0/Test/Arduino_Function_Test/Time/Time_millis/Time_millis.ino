unsigned long T_time;

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println("[Time]: Test millis()");
  Serial.print("Time:");
  T_time = millis();
  
  //print time
  Serial.println(T_time);
  Serial.println("[Time]: Done");
  delay(1000);  
}
