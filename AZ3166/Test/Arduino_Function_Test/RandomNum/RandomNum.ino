void setup(){
    Serial.begin(115200);
    randomSeed(analogRead(0));
}

long randNum;

void loop(){
    Serial.println("[RandomNumber]: Test random()");
    randNum = random(300);

    Serial.println(randNum);
    if(randNum >= 300)
    {
        Serial.println("[RandomNumber]: Error: random(300): The number should be from 0 to 200");
    }

    randNum = random(10,20);
    Serial.println(randNum);
    if(randNum < 10 || randNum >=20)
    {
        Serial.println("[RandomNumber]: Error: random(10,20): The number should be from 10 to 19");
    }

    Serial.println("[RandomNumber]: Done");
    delay(1000);
}
