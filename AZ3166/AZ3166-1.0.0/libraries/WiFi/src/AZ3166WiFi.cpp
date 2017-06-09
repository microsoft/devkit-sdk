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

#include <inttypes.h>

#include "AZ3166WiFi.h"
#include "wiring.h"
#include "utility/wl_types.h"
#include "SystemWiFi.h"
#include "utility/wl_definitions.h"
#include "EMW10xxInterface.h"
#include "EEPROMInterface.h"
#include "mico.h"
#include "telemetry.h"

int16_t WiFiClass::_state[MAX_SOCK_NUM] = { NA_STATE, NA_STATE, NA_STATE, NA_STATE };

WiFiClass::WiFiClass()
{
    firmware_version[0] = 0;
    ssid[0] = 0;
    ap_number = 0;
    is_station_inited = InitSystemWiFi();
    is_ap_inited = false;
    current_status = WL_IDLE_STATUS;
}

const char* WiFiClass::firmwareVersion()
{
    if (!is_station_inited) 
    {
        return NULL;
    }

    if (firmware_version[0] == 0)
    {
        // Initialize Wi-Fi driver befor get the version
        WiFiInterface()->get_mac_address();
        // Get version of the Wi-Fi firmware
        memset(firmware_version, 0, sizeof(firmware_version));
        int ret = MicoGetRfVer(firmware_version, sizeof(firmware_version));
        if (ret != 0)
        {
            firmware_version[0] = 0;
            return NULL;
        }
    }
    return firmware_version;
}

int WiFiClass::begin(void)
{
    if (!is_station_inited) 
    {
        return WL_CONNECT_FAILED;
    }

    if (SystemWiFiConnect())
    {
        strcpy(this->ssid, SystemWiFiSSID());
        current_status = WL_CONNECTED;
        return WL_CONNECTED;
    }

    current_status = WL_DISCONNECTED;
    return WL_CONNECT_FAILED;
}

int WiFiClass::begin(char* ssid)
{
    this->begin(ssid, NULL);
}

int WiFiClass::begin(char* ssid, const char *passphrase)
{
    if (!is_station_inited)
    {
        return WL_CONNECT_FAILED;
    }

    ((EMW10xxInterface*)WiFiInterface())->set_interface(Station);
    if (((EMW10xxInterface*)WiFiInterface())->connect(ssid, passphrase, NSAPI_SECURITY_WPA_WPA2, 0) == 0)
    {
        // Initialize the telemetry only after Wi-Fi established
        telemetry_init();
        send_telemetry_data("", "wifi", "Wi-Fi connected");
        strcpy(this->ssid, ssid);
        current_status = WL_CONNECTED;
        return WL_CONNECTED;
    }

    current_status = WL_DISCONNECTED;
    return WL_CONNECT_FAILED;
}

int WiFiClass::disconnect()
{
    if (is_station_inited)
    {
        ((EMW10xxInterface*)WiFiInterface())->set_interface(Station);
        WiFiInterface()->disconnect();
        is_station_inited = false;
    }
    disconnectAP();
    current_status = WL_DISCONNECTED;
    return WL_SUCCESS;
}

int WiFiClass::beginAP(char* ssid, const char *passphrase)
{
    if(is_ap_inited)
    {
        return WL_CONNECTED;
    }
    if (InitSystemWiFi())
    {
        is_ap_inited = SystemWiFiAPStart(ssid, passphrase);
        if (is_ap_inited)
        {
            strcpy(this->ssid, ssid);
            current_status = WL_CONNECTED;
            return WL_CONNECTED;
        }
    }
    current_status = WL_DISCONNECTED;
    return WL_CONNECT_FAILED;
}

int WiFiClass::disconnectAP()
{
    if (is_ap_inited)
    {
        Serial.println("disconnect AP");
        ((EMW10xxInterface*)WiFiInterface())->set_interface(Soft_AP);
        WiFiAPInterface()->disconnect();
        is_ap_inited = false;
    }
    current_status = WL_DISCONNECTED;
    return WL_SUCCESS;
}

uint8_t* WiFiClass::macAddress(uint8_t* mac)
{
    if (!is_station_inited)
    {
        return NULL;
    }
    // Initialize Wi-Fi driver first
    WiFiInterface()->get_mac_address();

    mico_wlan_get_mac_address(mac);
    return mac;
}

IPAddress WiFiClass::localIP()
{
    IPAddress ret;

    if (is_station_inited)
    {
        const char* ip = WiFiInterface()->get_ip_address();
        ret.fromString(ip);
    }
    return ret;
}

IPAddress WiFiClass::subnetMask()
{
    IPAddress ret;

    if (is_station_inited)
    {
        const char* mask = WiFiInterface()->get_netmask();
        ret.fromString(mask);
    }
    return ret;
}

IPAddress WiFiClass::gatewayIP()
{
    IPAddress ret;
    if (is_station_inited)
    {
        const char* ipgw = WiFiInterface()->get_gateway();
        ret.fromString(ipgw);
    }
    return ret;
}

const char* WiFiClass::SSID()
{
    return this->ssid;
}

uint8_t* WiFiClass::BSSID(uint8_t* bssid)
{
    return macAddress(bssid);
}

int32_t WiFiClass::RSSI()
{
    if (is_station_inited)
    {
        return ((EMW10xxInterface*)WiFiInterface())->get_rssi();
    }
}

uint8_t WiFiClass::encryptionType()
{
    return ENC_TYPE_CCMP;
}

int8_t WiFiClass::scanNetworks()
{
    if (!is_station_inited)
    {
        return WL_FAILURE;
    }

    memset(aps, 0, sizeof(aps));
    uint8_t attempts = sizeof(aps) / sizeof(aps[0]);

    ap_number = ((EMW10xxInterface*)WiFiInterface())->scan(aps, attempts);
    if(ap_number > 0)
    {
        current_status = WL_SCAN_COMPLETED;
    }
    return ap_number;
}


const char* WiFiClass::SSID(uint8_t networkItem)
{
    if (networkItem >= ap_number)
    {
        return NULL;
    }
	
    return aps[networkItem].get_ssid();
}

int32_t WiFiClass::RSSI(uint8_t networkItem)
{
    if (networkItem >= ap_number)
    {
        return NULL;
    }

    return aps[networkItem].get_rssi();
}


uint8_t WiFiClass::encryptionType(uint8_t networkItem)
{
    if (networkItem >= ap_number)
    {
        return ENC_TYPE_NONE;
    }

    nsapi_security_t ret = aps[networkItem].get_security();
    switch(ret)
    {
    case NSAPI_SECURITY_WEP:
        return ENC_TYPE_WEP;

    case NSAPI_SECURITY_WPA:
        return ENC_TYPE_TKIP;

    case NSAPI_SECURITY_WPA2:
        return ENC_TYPE_CCMP;

    case NSAPI_SECURITY_WPA_WPA2:
        return ENC_TYPE_AUTO;

    default:
        return ENC_TYPE_NONE;
    }
}


uint8_t WiFiClass::status()
{
    return current_status;
}

WiFiClass WiFi;

