/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef Arduino_h
#define Arduino_h

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "mbed.h"
#include "mbed_stats.h"
#include "PinNames.h"

#include "AttachInterrupt.h"
#include "EEPROMInterface.h"
#include "floatIO.h" 
#include "Stream.h"	
#include "SystemFunc.h"
#include "SystemVersion.h"
#include "SystemTickCounter.h"
#include "SystemVariables.h"
#include "SystemWeb.h"
#include "Watchdog.h"
#include "WCharacter.h"
#include "wiring.h"
#include "wiring_analog.h"
#include "wiring_digital.h"
#include "WMath.h"
#include "WString.h"

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#define F_CPU SystemCoreClock //To be compatible with Arduino delay

#define clockCyclesPerMicrosecond() ( SystemCoreClock / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (SystemCoreClock / 1000L) )
#define microsecondsToClockCycles(a) ( (a) * (SystemCoreClock / 1000000L) )

// Sketch
extern void setup(void) ;
extern void loop(void) ;

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

// Arduino objects - C++ only
#ifdef __cplusplus
#include "UARTClass.h"
#include "OledDisplay.h"

extern UARTClass Serial;
extern OLEDDisplay Screen;
#endif // __cplusplus

// SPI interface
#define SPI_INTERFACES_COUNT    1

#endif // Arduino_h