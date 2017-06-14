#define LOOP_DELAY          500

int counter = 1;

typedef struct BytesData{
    unsigned int test_data;
    byte lb;
    byte hb;
}BytesData;

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
    Serial.println("Verify bits()");
    check_bits();

    Serial.println("Verify bytes()");
    check_bytes();

    delay(LOOP_DELAY);
}

void check_bits()
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
}

void check_bytes()
{
    BytesData bytes[3]= {
      {1022, 254, 3}, 
      {1, 1, 0},  
      {2048,0,8}
    };
    byte lb, hb;

    for(int i=0; i<3; ++i)
    {
      //LowByte returns the low Byte(that is righmost) of a variable(e.g.a word)
      lb= lowByte(bytes[i].test_data);
      if(lb != bytes[i].lb)
      {
        Serial.printf("Error: lowByte(): For %d, the low byte should be %d", bytes[i].test_data, bytes[i].lb);
        Serial.println();
      }

      //HighByte returns the higher Byte(that is leftmost) of a variable to 2byte(16bit) or the second lowest byte of a larger data type.
      hb= highByte(bytes[i].test_data);
      if(hb != bytes[i].hb)
      {
        Serial.printf("Error: highByte(): For %d, the high byte should be %d", bytes[i].test_data, bytes[i].hb);
        Serial.println();
      }
    }
}
