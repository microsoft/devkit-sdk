#ifndef __RGB_LED_H__
#define __RGB_LED_H__

#include "mbed.h"

class RGB_LED
{
    public:
        RGB_LED(PinName red = PB_4, PinName green = PB_3, PinName blue = PC_7);

        void setColor(int red = 255, int green = 255, int blue = 255);
        void turnOff();

    private: 
        PwmOut _red;
        PwmOut _green;
        PwmOut _blue;
};

#endif