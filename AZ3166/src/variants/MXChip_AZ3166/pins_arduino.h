/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __PINS_ARDUINO_IOT_DEVKIT_H__
#define __PINS_ARDUINO_IOT_DEVKIT_H__

#include "PinNames.h"

// Micro：Bit Edge pins
#define PIN_0          PB_0;   // GPIO, ADC, SPI CS
#define PIN_4          PA_5;   // GPIO, ADC
#define PIN_5          PA_4;   // GPIO, ADC
#define PIN_6          PB_4;   // GPIO, PWM
#define PIN_7          PB_3;   // GPIO, PWM
#define PIN_1          PB_6;   // GPIO, UART TX
#define PIN_8          PC_13;  // GPIO
#define PIN_9          PB_10;  // GPIO
#define PIN_10         PC_7;   // GPIO, PWM
#define PIN_11         PA_10;  // GPIO
#define PIN_12         PB_2;   // GPIO
#define PIN_2          PB_7;   // GPIO, UART RX
#define PIN_13         PB_13;  // GPIO, SPI SCK
#define PIN_14         PB_14;  // GPIO, SPI MISO
#define PIN_15         PB_15;  // GPIO, SPI MOSI
#define PIN_16         PC_6;   // GPIO
#define PIN_19         PB_8;   // GPIO, I2C SCL
#define PIN_20         PB_9;   // GPIO, I2C SDA

// Arduino pins are defined in PinNames.h

#endif /* _VARIANT_ARDUINO_STM32_ */
