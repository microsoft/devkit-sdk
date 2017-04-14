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
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"

NetworkInterface *network;

static void InitWiFi(void)
{
    network = new EMW10xxInterface();
    
    EEPROMInterface eeprom;
    
    uint8_t ssid[WIFI_SSID_MAX_LEN + 1] = { '\0' };
    uint8_t pwd[WIFI_PWD_MAX_LEN + 1] = { '\0' };
    
    int ret = eeprom.read(ssid, WIFI_SSID_MAX_LEN, WIFI_SSID_ZONE_IDX);
    if (ret < 0)
    {
        Serial.print("ERROR: Failed to get the Wi-Fi SSID from EEPROM.\r\n");
        return;
    }
    else if(ret == 0)
    {
        Serial.print("INFO: the Wi-Fi SSID is empty, please set the value in configuration mode.\r\n");
        return;
    }
    ret = eeprom.read(pwd, WIFI_PWD_MAX_LEN, WIFI_PWD_ZONE_IDX);
    if (ret < 0)
    {
        Serial.print("ERROR: Failed to get the Wi-Fi password from EEPROM.\r\n");
        return;
    }
    
    ret = ((EMW10xxInterface*)network)->connect( (char*)ssid, (char*)pwd, NSAPI_SECURITY_WPA_WPA2, 0 );
    if(ret != 0)
    {
      	Serial.printf("ERROR: Failed to connect Wi-Fi %s.\r\n", ssid);
    }
    
    Serial.printf("Wi-Fi %s connected.\r\n", ssid);
}

static void TryConfigurationiMode()
{
    pinMode(USER_BUTTON_A, INPUT);

    int buttonState = digitalRead(USER_BUTTON_A);
    if(buttonState == LOW)
    {
        Screen.clean();
        Screen.print("Azure IoT DevKit\r\n \r\nConfiguration\r\n");
    
        // Enter configuration mode
        cli_main();
    }
    else
    {
        Screen.print("Azure IoT DevKit\r\n \r\nRunning...\r\n");
        Serial.print("You can press Button A and reset to enter configuration mode.\r\n");
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
    
    initVariant();
    
#if defined(USBCON)
    USBDevice.attach();
#endif
    
    Serial.print("\r\n************************************************");
    Serial.print("\r\n** MXChip - Microsoft Azure IoT Developer Kit **");
    Serial.print("\r\n************************************************\r\n");
    
    // Initialize the OLED screen
    Screen.init();

    // Enable Wi-Fi
    InitWiFi();

    // Check running mode
    TryConfigurationiMode();
    
    // Arduino setup function
    Serial.print("\r\n\r\n");
    setup();

    for (;;)
    {
        // Arduino loop function
        loop();
    }
    
    return 0;
}
