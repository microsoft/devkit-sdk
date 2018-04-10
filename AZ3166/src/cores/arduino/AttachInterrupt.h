#ifndef __ATTACH_INTERRUPT_H__
#define __ATTACH_INTERRUPT_H__

#include "mbed.h"

/**
* @brief    Attach interrupt callback to digital pin.
**
* @param    pin               The PinName of digital pin.
* @param    ISR               The callback function.
* @param    mode              Triggered event type, support CHANGE, RISING, FALLING now.
*
* @return   Return 0 on success. Return -1 on fail.
*           The failure might be caused by unsupported PinName.
*/
int attachInterrupt(PinName pin, Callback<void()> ISR, int mode);

#endif