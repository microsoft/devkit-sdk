/**
 ******************************************************************************
 * The MIT License (MIT)
 * Copyright (C) 2017 Microsoft Corp. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

 
#include "Arduino.h"
#include "SystemWiFi.h"
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"
#include "NTPClient.h"
#include "telemetry.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// WiFi related functions
NetworkInterface *network = NULL;
static char ssid[WIFI_SSID_MAX_LEN + 1] = { 0 };

bool InitSystemWiFi(void)
{
    if (network == NULL)
    {
        network = new EMW10xxInterface();
    }
    
    return (network != NULL);
}

bool SystemWiFiConnect(void)
{
    EEPROMInterface eeprom;
    
    uint8_t pwd[WIFI_PWD_MAX_LEN + 1] = { '\0' };
    
    int ret = eeprom.read((uint8_t*)ssid, WIFI_SSID_MAX_LEN, 0x00, WIFI_SSID_ZONE_IDX);
    if (ret < 0)
    {
        Serial.print("ERROR: Failed to get the Wi-Fi SSID from EEPROM.\r\n");
        return false;
    }
    else if(ret == 0)
    {
        Serial.print("INFO: the Wi-Fi SSID is empty, please set the value in configuration mode.\r\n");
        return false;
    }
    ret = eeprom.read(pwd, WIFI_PWD_MAX_LEN, 0x00, WIFI_PWD_ZONE_IDX);
    if (ret < 0)
    {
        Serial.print("ERROR: Failed to get the Wi-Fi password from EEPROM.\r\n");
        return false;
    }
    
    ret = ((EMW10xxInterface*)network)->connect( (char*)ssid, (char*)pwd, NSAPI_SECURITY_WPA_WPA2, 0 );
    if(ret != 0)
    {
      	Serial.printf("ERROR: Failed to connect Wi-Fi %s.\r\n", ssid);
        return false;
    }
    else
    {
        Serial.printf("Wi-Fi %s connected.\r\n", ssid);
        // Initialize the telemetry only after Wi-Fi established
        telemetry_init();
        
        send_telemetry_data("", "wifi", "Wi-Fi connected");

        return true;
    }
}

const char* SystemWiFiSSID(void)
{
    return ssid;
}

NetworkInterface* WiFiInterface(void)
{
    return network;
}

int WiFiScan(WiFiAccessPoint *res, unsigned count)
{
    if (network != NULL)
    {
        return ((EMW10xxInterface*)network)->scan(res, count);
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
// WiFi AP related functsion.
bool InitSystemWiFiAP(void)
{
    if (network == NULL)
    {
        InitSystemWiFi();
    }
    
    if (network != NULL)
    {
         ((EMW10xxInterface*)network)->set_interface(Soft_AP);
         return true;
    }

    return false;
}

bool SystemWiFiAPStart(const char *ssid, const char *passphrase)
{
    if (network != NULL)
    {
        int ret = ((EMW10xxInterface*)network)->connect( (char*)ssid, (char*)passphrase, NSAPI_SECURITY_WPA_WPA2, 0 );
        if(ret != 0)
        {
            Serial.printf("ERROR: Failed to start AP for Wi-Fi %s.\r\n", ssid);
            return false;
        }
        else
        {
            Serial.printf("AP mode Wi-Fi %s started .\r\n", ssid);
            return true;
        }
    }

    return false;
}

NetworkInterface* WiFiAPInterface(void)
{
    return network;
}