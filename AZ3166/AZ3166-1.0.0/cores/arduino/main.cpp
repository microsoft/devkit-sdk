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
#include "console_cli.h"
#include "SystemWiFi.h"
#include "mbed_stats.h"

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant(){ }

static bool Initialization(void)
{
    // Initialize watchdog
    //watchdogSetup();
    
    initVariant();

    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);

#if defined(USBCON)
    USBDevice.attach();
#endif
    
    Serial.print("\r\n************************************************");
    Serial.print("\r\n** MXChip - Microsoft Azure IoT Developer Kit **");
    Serial.print("\r\n************************************************\r\n");
    
    // Initialize the OLED screen
    Screen.init();

    return true;
}

static bool IsConfigurationMode()
{
    pinMode(USER_BUTTON_A, INPUT);
    int buttonState = digitalRead(USER_BUTTON_A);
    if(buttonState == LOW)
    {
        return true;
    }
    return false;
}

static void EnterConfigurationiMode()
{
    pinMode(USER_BUTTON_A, INPUT);

    Screen.print("Azure IoT DevKit\r\n \r\nConfiguration\r\n");

    if (!InitSystemWiFi())
    {
        return;
    }
    
    const char* mac = WiFiInterface()->get_mac_address();
    
    char m[20] = { '\0'};
    m[0] = 'i';
    m[1] = 'd';
    m[2] = ':';
    for(int i =0, j = 3; i < strlen(mac); i++)
    {
        if (mac[i] != ':')
        {
            m[j++] = mac[i];
        }
    }
    Screen.print(1, m);

    // Enter configuration mode
    cli_main();
}

static void EnterUserMode()
{
    Serial.print("You can press Button A and reset to enter configuration mode.\r\n\r\n");
    
    // Arduino setup function
    setup();

    for (;;)
    {
        // Arduino loop function
        loop();
    }
}

/*
 * \brief Main entry point of Arduino application
 */
int main( void )
{
    Initialization();

    if (IsConfigurationMode())
    {
        EnterConfigurationiMode();
    }
    else
    {
        EnterUserMode();
    }
    
    return 0;
}
