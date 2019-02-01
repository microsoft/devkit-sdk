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
#include "mico.h"

#include <inttypes.h>

#include "AZ3166WiFi.h"
#include "EEPROMInterface.h"
#include "EMW10xxInterface.h"
#include "SystemTime.h"
#include "SystemWiFi.h"
#include "Telemetry.h"
#include "utility/wl_definitions.h"
#include "utility/wl_types.h"
#include "wiring.h"

int16_t WiFiClass::_state[MAX_SOCK_NUM] = { NA_STATE, NA_STATE, NA_STATE, NA_STATE };

WiFiClass::WiFiClass()
{
    firmware_version[0] = 0;
    ssid[0] = 0;
    ap_number = 0;
    is_wifi_inited = InitSystemWiFi();
    is_station_inited = false;
    is_ap_inited = false;
    current_status = WL_IDLE_STATUS;
}

const char* WiFiClass::firmwareVersion()
{
    if (!is_wifi_inited) 
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
    if (!is_wifi_inited) 
    {
        return WL_CONNECT_FAILED;
    }

    if(is_station_inited)
    {
        return WL_CONNECTED;
    }

    if (SystemWiFiConnect())
    {
        strcpy(this->ssid, SystemWiFiSSID());
        is_station_inited = true;
        current_status = WL_CONNECTED;
        return WL_CONNECTED;
    }

    current_status = WL_DISCONNECTED;
    return WL_CONNECT_FAILED;
}

int WiFiClass::begin(char* ssid)
{
    return this->begin(ssid, NULL);
}

int WiFiClass::begin(char* ssid, const char *passphrase)
{
    if (!is_wifi_inited)
    {
        return WL_CONNECT_FAILED;
    }

    if(is_station_inited)
    {
        return WL_CONNECTED;
    }

    ((EMW10xxInterface*)WiFiInterface())->set_interface(Station);
    if (((EMW10xxInterface*)WiFiInterface())->connect(ssid, passphrase, NSAPI_SECURITY_WPA_WPA2, 0) == 0)
    {
        // Sync system from NTP time server
        SyncTime();

        // Initialize the telemetry only after Wi-Fi established
        telemetry_init();
        
        // Microsoft collects data to operate effectively and provide you the best experiences with our products. 
        // We collect data about the features you use, how often you use them, and how you use them.
        send_telemetry_data_async("", "wifi", "Wi-Fi connected");

        strcpy(this->ssid, ssid);
        is_station_inited = true;
        current_status = WL_CONNECTED;
        return WL_CONNECTED;
    }

    current_status = WL_DISCONNECTED;
    return WL_CONNECT_FAILED;
}

int WiFiClass::disconnect()
{
    if (!is_wifi_inited) 
    {
        return WL_CONNECT_FAILED;
    }

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

unsigned char* WiFiClass::macAddress(unsigned char* mac)
{
    if (!is_wifi_inited)
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

    if (is_wifi_inited)
    {
        const char* ip = WiFiInterface()->get_ip_address();
        ret.fromString(ip);
    }
    return ret;
}

IPAddress WiFiClass::subnetMask()
{
    IPAddress ret;

    if (is_wifi_inited)
    {
        const char* mask = WiFiInterface()->get_netmask();
        ret.fromString(mask);
    }
    return ret;
}

IPAddress WiFiClass::gatewayIP()
{
    IPAddress ret;
    if (is_wifi_inited)
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

unsigned char* WiFiClass::BSSID(unsigned char* bssid)
{
    return macAddress(bssid);
}

int WiFiClass::RSSI()
{
    if (is_wifi_inited)
    {
        return ((EMW10xxInterface*)WiFiInterface())->get_rssi();
    }
    return 0;
}

int WiFiClass::encryptionType()
{
    return ENC_TYPE_CCMP;
}

int WiFiClass::scanNetworks()
{
    if (!is_wifi_inited)
    {
        return WL_FAILURE;
    }

    memset(aps, 0, sizeof(aps));
    int attempts = sizeof(aps) / sizeof(aps[0]);

    ap_number = ((EMW10xxInterface*)WiFiInterface())->scan(aps, attempts);
    if(ap_number > 0)
    {
        current_status = WL_SCAN_COMPLETED;
    }
    return ap_number;
}

const char* WiFiClass::SSID(unsigned char networkItem)
{
    if (networkItem >= ap_number)
    {
        return NULL;
    }

    return aps[networkItem].get_ssid();
}

int WiFiClass::RSSI(unsigned char networkItem)
{
    if (networkItem >= ap_number)
    {
        return 0;
    }

    return aps[networkItem].get_rssi();
}


int WiFiClass::encryptionType(unsigned char networkItem)
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


unsigned char WiFiClass::status()
{
    if (SystemWiFiRSSI() == 0)
    {
        return ((current_status == WL_CONNECTED) ? WL_CONNECTION_LOST : WL_DISCONNECTED);
    }
    return current_status;
}

WiFiClass WiFi;

