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

NetworkInterface *network = NULL;

bool InitSystemWiFi(bool connect)
{
    network = new EMW10xxInterface();

    if (!connect)
    {
        return true;
    }
    
    EEPROMInterface eeprom;
    
    uint8_t ssid[WIFI_SSID_MAX_LEN + 1] = { '\0' };
    uint8_t pwd[WIFI_PWD_MAX_LEN + 1] = { '\0' };
    
    int ret = eeprom.read(ssid, WIFI_SSID_MAX_LEN, 0x00, WIFI_SSID_ZONE_IDX);
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