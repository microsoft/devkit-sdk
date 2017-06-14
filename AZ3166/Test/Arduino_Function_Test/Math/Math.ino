#define LOOP_DELAY          500
#define Math_PositiveNum    10
#define Math_Zero           0
#define Math_NegativeNum    -1

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

void runCase(){
    Serial.println("Verify abs()");
    check_mathabs();

    Serial.println("Verify constrain()");
    check_mathconstrain();

    Serial.println("Verify map()");
    check_mathmap();

    Serial.println("Verify max()");
    check_mathmax();

    Serial.println("Verify min()");
    check_mathmin();

    Serial.println("Verify pow()");
    check_mathpow();

    Serial.println("Verify sqrt()");
    check_mathsqrt();
}

void check_mathabs()
{
    if(abs(Math_PositiveNum) != Math_PositiveNum )
    {
        Serial.println("Error: Math.abs(): If x is greater than 0, the absolute value should be equal to x");
    }

    if(abs(Math_Zero) != Math_Zero )
    {
        Serial.println("Error: Math.abs(): If number is 0, the absolute value should be 0");
    }

    if(abs(Math_NegativeNum) != Math_NegativeNum * (-1) )
    {
        Serial.println("Error: Math.abs(): If x is less than 0, the absolute value should be equal to -x");
    }
    
    delay(LOOP_DELAY);
}

void check_mathconstrain()
{
    if(constrain(Math_NegativeNum, Math_Zero, Math_PositiveNum) != Math_Zero)
    {
        Serial.println("Error: Math.constrain(): If testVal is less than oneNum, oneNum should be return"); 
    }

    if(constrain(Math_Zero, Math_NegativeNum, Math_PositiveNum) != Math_Zero)
    {
        Serial.println("Error: Math.constrain(): If testVal is between oneNum and secondNum, testVal should be return"); 
    }

    if(constrain(Math_PositiveNum, Math_NegativeNum, Math_Zero) != Math_Zero)
    {
        Serial.println("Error: Math.constrain(): If testVal is greater than secondNum, secondNum should be return"); 
    }  

    delay(LOOP_DELAY);
}

void check_mathmap()
{
    int val = analogRead(0);
    val = map(val,0,1023,0,255);

    Serial.println(val);
    if(val < 0 || val > 255)
    {
      Serial.println("Error: map(val,0,1023,0,255): The value shoud be between 0 and 255");
    }

    delay(LOOP_DELAY);
}

void check_mathmax()
{
    int maxVal = max(Math_NegativeNum,Math_PositiveNum);

    if(maxVal != Math_PositiveNum)
    {
        Serial.println("Error: Math.max(): Failed to get the max number.");
    }    

    delay(LOOP_DELAY); 
}

void check_mathmin()
{
    int minVal = min(Math_NegativeNum,Math_PositiveNum);

    if(minVal != Math_NegativeNum)
    {        
        Serial.println("Error: Math.min(): Failed to get the min number.");
    }    

    delay(LOOP_DELAY);
}

void check_mathpow()
{
    if(pow(2,3) != 8)
    {
      Serial.println("Error: Math.pow(): Failed to get the result of 2^3");
    }

    if(pow(9,0.5) != 3)
    {
      Serial.println("Error: Math.pow(): Failed to get the result of 9^0.5");
    }

    delay(LOOP_DELAY); 
}

void check_mathsqrt()
{
    if(sqrt(9) != 3)
    {
      Serial.println("Error: Math.sqrt(): Failed to get the square root of 9");
    }

    if(sqrt(1.0) != 1)
    {
      Serial.println("Error: Math.sqrt(): Failed to get the square root of 1");
    }
    
    delay(LOOP_DELAY);
}
