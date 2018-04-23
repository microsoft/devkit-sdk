#include "AttachInterrupt.h"
#include "wiring_constants.h"

static PinName validPinames[16] = {PB_0, NC, PB_2, PB_3, PA_4, PA_5, PB_6, PB_7, PB_8, PB_9, PA_10, NC, NC, PB_13, PB_14, PB_15};
static InterruptIn event[16] = {InterruptIn(PB_0), InterruptIn(NC), InterruptIn(PB_2), InterruptIn(PB_3), 
                         InterruptIn(PA_4), InterruptIn(PA_5), InterruptIn(PB_6), InterruptIn(PB_7), 
                         InterruptIn(PB_8), InterruptIn(PB_9), InterruptIn(PA_10), InterruptIn(NC), 
                         InterruptIn(NC), InterruptIn(PB_13), InterruptIn(PB_14), InterruptIn(PB_15)};

int attachInterrupt(PinName pin, Callback<void()> ISR, int mode)
{
    if (pin == NC || validPinames[pin & 0x0F] != pin)
    {
        return -1;
    }
    if (mode == CHANGE)
    {
        event[pin & 0x0F].rise(ISR);
        event[pin & 0x0F].fall(ISR);
    }
    if (mode == RISING)
    {
        event[pin & 0x0F].rise(ISR);
    }
    if (mode == FALLING)
    {
        event[pin & 0x0F].fall(ISR);
    }
    return 0;
}

int detachInterrupt(PinName pin)
{
    if (pin == NC || validPinames[pin & 0x0F] != pin)
    {
        return -1;
    }
    event[pin & 0x0F].rise(NULL);
    event[pin & 0x0F].fall(NULL);
    return 0;
}