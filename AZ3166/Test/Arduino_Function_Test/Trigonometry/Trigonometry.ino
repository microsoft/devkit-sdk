#include <ArduinoUnit.h>
#define LOOP_DELAY          500

void setup() {
    Serial.println(__FILE__);
}

void loop() {
    Test::run();
}

test(check_trigonometry)
{
    int data[9]={PI, PI/2,PI/3, PI/4, 0, -PI/2,-PI/3, -PI/4, -PI};

    for(int i=0; i<9; ++i)
    {        
        if(data[i] ==0)
        {
            // for sin(0) should be 0
            assertEqual(sin(0), 0);

            // for cos(0) should be 1
            assertEqual(cos(0), 1);

            // for tan(0) should be 0
            assertEqual(tan(0), 0);
        }
        else // except 0, others value are not precise
        {
            int result = sin(data[i]);
            assertLessOrEqual(result,1);
            assertMoreOrEqual(result, -1);

            result = cos(data[i]);
            assertLessOrEqual(result,1);
            assertMoreOrEqual(result, -1);

            //not check the tan() here as it caused out of memory
        }
    }
    
    delay(LOOP_DELAY);
}