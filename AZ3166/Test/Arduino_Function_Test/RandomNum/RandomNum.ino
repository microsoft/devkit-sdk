#define LOOP_DELAY          500

int counter = 1;
long randNum;

void setup(){
    Serial.println(">> Start");
    Serial.println(__FILE__); 

    randomSeed(analogRead(0));
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
    randNum = random(300);

    Serial.println(randNum);
    if(randNum >= 300)
    {
        Serial.println("Error: random(300): The number should be from 0 to 200");
    }

    randNum = random(10,20);
    Serial.println(randNum);
    if(randNum < 10 || randNum >=20)
    {
        Serial.println("Error: random(10,20): The number should be from 10 to 19");
    }

    delay(LOOP_DELAY);
}
