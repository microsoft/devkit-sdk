#define LOOP_DELAY          500

int counter = 1;
int incomingByte = 0;

void setup()
{
    Serial.println(">> Start");
    Serial.println(__FILE__);    
}

void loop() {
  while(counter <= 5)
  {
    Serial.printf(">> Start (%d)\r\n", counter);
    runCase();
    Serial.printf(">> End (%d)\r\n", counter); 

    if(counter == 5)
    {
      Serial.println();
      Serial.println(">> End");
    }
    
    counter++;
  }
}

void runCase()
{
    Serial.println(Serial.available());

    incomingByte = Serial.read();
    Serial.println(incomingByte);

    Serial.end();
    Serial.println("Test serial.end()");
    Serial.begin(115200);
    Serial.println("Test serial.begin()");

    Serial.println("Please enter some string which contain 'test'");
    if(!Serial.find("test"))
    {
      Serial.println("Error: failed to find the string test");
    }

    if(!Serial.findUntil("test","#"))
    {
      Serial.println("Error: failed to find the string test");
    }

    Serial.flush();

    //long result = Serial.parseInt();
    //Serial.println(result);

    //float resultFloat = Serial.parseFloat();
    //Serial.println(resultFloat);

    Serial.println(Serial.peek());    

    delay(LOOP_DELAY);
}
