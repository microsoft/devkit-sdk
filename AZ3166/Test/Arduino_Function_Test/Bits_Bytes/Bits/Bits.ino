#define LOOP_DELAY          500

int counter = 1;

void setup(){
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
    int value = 22;

    int x = bitRead(value,3);
    Serial.println(x);
    if(x!=0)
    {
        Serial.println("Error: bitRead(22,3): 22 in binary is 0010110, the 3rd bit is 0");        
    }

    x=bitWrite(value,3,1);
    Serial.println(x);
    if(x!=30)
    {
        Serial.println("Error: bitWrite(22,3,1): it should be 30 after change the 3rd bit to 1");
    }

    x = bitSet(value,3); 
    Serial.println(x);
    if(x!=30)
    {
        Serial.println("Error: bitSet(22,3): it should be 30 after change the 3rd bit to 1");
    }

    x = bitClear(value, 2);
    Serial.println(x);
    if(x!=26)
    {
        Serial.println("Error: bitClear(22,2): it should be 26 after clean the 2nd bit");
    }

    x= bit(2);
    Serial.println(x);
    if(x!=4)
    {
        Serial.println("Error: bit(2): it should be 4");
    }

    delay(LOOP_DELAY);
}
