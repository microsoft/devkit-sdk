#ifndef __ATTACH_INTERRUPT_H__
#define __ATTACH_INTERRUPT_H__

#include "mbed.h"

/**
* @brief    Attach interrupt callback to digital pin.
**
* @param    pin               The PinName of digital pin.
*                             Valid PinNames are: PA_4, PA_5, PA_10, PB_0, PB_2, PB_3, PB_6,
*                                                 PB_7, PB_8, PB_9, PB_13, PB_14, PB_15
* @param    ISR               The callback function.
* @param    mode              Triggered event type, support CHANGE, RISING, FALLING now.
*
* @return   Return 0 on success. Return -1 on fail.
*           The failure might be caused by unsupported PinName.
*/
int attachInterrupt(PinName pin, Callback<void()> ISR, int mode);

/**
* @brief    Detach interrupt callback from digital pin.
**
* @param    pin               The PinName of digital pin.
*                             Valid PinNames are: PA_4, PA_5, PA_10, PB_0, PB_2, PB_3, PB_6,
*                                                 PB_7, PB_8, PB_9, PB_13, PB_14, PB_15
*
* @return   Return 0 on success. Return -1 on fail.
*           The failure might be caused by unsupported PinName.
*/
int detachInterrupt(PinName pin);

#endif