unsigned long T_time;

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("[Time]: Test micros()");
  Serial.print("Time:");
  T_time = micros();
  
  //print time
  Serial.println(T_time);
  Serial.println("[Time]: Done");
  delay(1000);  
}
