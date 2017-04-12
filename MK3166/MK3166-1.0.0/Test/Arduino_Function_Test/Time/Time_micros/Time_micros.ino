unsigned long T_time;

void setup() {
  Serial.begin(9600);
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
