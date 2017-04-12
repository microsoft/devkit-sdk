void setup() 
{    
    Serial.begin(9600);
    pinMode(ARDUINO_PIN_A3, OUTPUT);
}

int val = 0;

void loop() {  
  Serial.println("[Analog I/O]: Test analyWriteResolution()");
  val= analogRead(ARDUINO_PIN_A2);
  Serial.print("Analog Read:");
  Serial.println(val);

  analogWriteResolution(8);
  analogWrite(ARDUINO_PIN_A3, map(val,0,1023, 0, 255));
  Serial.print("8-bit PWM value:");
  Serial.println(map(val,0,1023, 0, 255));

  analogWriteResolution(12);
  analogWrite(ARDUINO_PIN_A3, map(val,0,1023, 0, 4095));
  Serial.print("12-bit PWM value:");
  Serial.println(map(val,0,1023, 0, 4095));

  analogWriteResolution(4);
  analogWrite(ARDUINO_PIN_A3, map(val,0,1023, 0, 15));
  Serial.print("4-bit PWM value:");
  Serial.println(map(val,0,1023, 0, 15));

  Serial.println("[Analog I/O]: Done");
  delay(1000);
}
