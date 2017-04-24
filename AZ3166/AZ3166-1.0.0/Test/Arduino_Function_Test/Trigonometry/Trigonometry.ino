void setup() {
  Serial.begin(115200);
}

void loop() {  
    Serial.println("[Trigonometry]: Test sin(), cos() and tan()");

    int data[9]={PI, PI/2,PI/3, PI/4, 0, -PI/2,-PI/3, -PI/4, -PI};

    for(int i=0; i<9; ++i)
    {        
        if(data[i] ==0)
        {
            if(sin(0) != 0)
            {
                Serial.println("[Trigonometry]: Error: trigonometry.sin(): for sin(0) should be 0");
            }

            if(cos(0) != 1)
            {
                Serial.println("[Trigonometry]: Error: trigonometry.cos(): for cos(0) should be 1");
            }

            if(tan(0) != 0)
            {
                Serial.println("[Trigonometry]: Error: trigonometry.tan(): for tan(0) should be 0");
            }
        }
        else // except 0, others value are not precise
        {
            if(sin(data[i]) < -1 || sin(data[i]) > 1)
            {
                Serial.println("[Trigonometry]: Error: trigonometry.sin(): the result should be between -1 and 1");
            }

            if(cos(data[i]) < -1 || cos(data[i]) > 1)
            {
                Serial.println("[Trigonometry]: Error: trigonometry.cos(): the result should be between -1 and 1");
            }

            //not check the tan() here as it caused out of memory
        }
    }
    
    Serial.println("[Trigonometry]: Done");
    delay(1000);
}
