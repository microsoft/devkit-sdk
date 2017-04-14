/*
  main.cpp - Main loop for Arduino sketches
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

#define ARDUINO_MAIN
#include "Arduino.h"
#include "mico_system.h"
#include "cli\console_cli.h"
#include "telemetry.h"

static void TryConfigurationiMode()
{
    pinMode(USER_BUTTON_A, INPUT);

    int buttonState = digitalRead(USER_BUTTON_A);
    if(buttonState == LOW)
    {
        // Enter configuration mode
         cli_main();
    }
    else
    {
        Serial.println("\r\nPlease press Button A and reset to enter configuration mode.");
    }
}

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant(){ }
/*
 * \brief Main entry point of Arduino application
 */
int main( void )
{
    // Initialize watchdog
    //watchdogSetup();
    
    //init();
    initVariant();
    
#if defined(USBCON)
    USBDevice.attach();
#endif
    
    TryConfigurationiMode();
    
    // Arduino setup function
    setup();
    telemetry_init()

    for (;;)
    {
        // Arduino loop function
        loop();
    }
    
    return 0;
}
