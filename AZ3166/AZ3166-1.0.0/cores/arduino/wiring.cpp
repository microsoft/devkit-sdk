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

#include "mbed.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t millis(void) {
    return us_ticker_read() / 1000;
}

// Interrupt-compatible version of micros
uint32_t micros( void )
{
    return us_ticker_read();
}


void delay( uint32_t ms )
{
  if (ms == 0)
      return;
  wait_ms(ms);
}


void delayMicroseconds( uint32_t us )
{
  if (us == 0)
      return;
  wait_us(us);
}

#ifdef __cplusplus
}
#endif
