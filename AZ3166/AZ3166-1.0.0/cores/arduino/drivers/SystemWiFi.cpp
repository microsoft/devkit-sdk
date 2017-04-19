/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Arduino.h"
#include "SystemWiFi.h"
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"
#include "NTPClient.h"
#include "telemetry.h"

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
        return true;
    }
    // Initialize the telemtry only after Wi-Fi established
    telemetry_init();
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

void SyncTime(void)
{
    NTPClient ntp(*network);
    ntp.setTime("0.pool.ntp.org");
}

////////////////////////////////////////////////////////////////////////////////////
// As EMW10xxInterface doesn't expose all functions for WiFi AP,
// here is the temp wrap for WiFi AP.
bool InitSystemWiFiAP(void)
{
    return false;
}

bool SystemWiFiAPStart(const char *ssid, const char *passphrase)
{
    return false;
}

NetworkInterface* WiFiAPInterface(void)
{
    return NULL;
}