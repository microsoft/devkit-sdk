#include <ArduinoUnit.h>
#define LOOP_DELAY          500

typedef struct BytesData{
    unsigned int test_data;
    byte lb;
    byte hb;
}BytesData;

void setup(){
  Serial.println(__FILE__);
}

void loop() {
    Test::run();
}

test(check_bits)
{
    int value = 22;

    int x = bitRead(value,3);
    assertEqual(x,0);

    x=bitWrite(value,3,1);
    assertEqual(x,30);

    x = bitSet(value,3); 
    assertEqual(x,30);

    x = bitClear(value, 2);
    assertEqual(x,26);

    x= bit(2);
    assertEqual(x,4);
}

test(check_bytes)
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
      assertEqual(lb, bytes[i].lb);

      //HighByte returns the higher Byte(that is leftmost) of a variable to 2byte(16bit) or the second lowest byte of a larger data type.
      hb= highByte(bytes[i].test_data);
      assertEqual(hb, bytes[i].hb);
    }
}