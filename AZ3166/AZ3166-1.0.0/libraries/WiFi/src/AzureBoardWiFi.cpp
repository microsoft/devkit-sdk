/*
  WiFi.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

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

#include <inttypes.h>

#include "mico.h"
#include "AzureBoardWiFi.h"
#include "utility\wifi_drv.h"
#include "wiring.h"
#include "utility\wl_types.h"
#include "SystemWiFi.h"
#include "wl_definitions.h"

int16_t WiFiClass::_state[MAX_SOCK_NUM] = { NA_STATE, NA_STATE, NA_STATE, NA_STATE };

WiFiClass::WiFiClass()
{
	firmware_version[0] = 0;
	ssid[0] = 0;
	ap_number = 0;
	is_station_inited = InitSystemWiFi();
	is_ap_inited = false;
	current_status = WL_NO_SHIELD;
}

WiFiClass::~WiFiClass()
{

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
		ret = MicoGetRfVer(firmware_version, sizeof(firmware_version));
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

	if (((EMW10xxInterface*)WiFiInterface())->connect(ssid, passphrase, NSAPI_SECURITY_WPA_WPA2, 0) == 0)
	{
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
		WiFiInterface()->disconnect();
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
	if (InitSystemWiFiAP())
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
	if (!is_station_inited)
	{
		return NULL;
	}
	const char* ip = WiFiInterface()->get_ip_address();

	IPAddress ret;
	ret.fromString(ip);
	return ret;
}

IPAddress WiFiClass::subnetMask()
{
	if (!is_station_inited)
	{
		return NULL;
	}
	const char* mask = WiFiInterface()->get_netmask();

	IPAddress ret;
	ret.fromString(mask);
	return ret;
}

IPAddress WiFiClass::gatewayIP()
{
	if (!is_station_inited)
	{
		return NULL;
	}
	const char* ipgw = WiFiInterface()->get_gateway();

	IPAddress ret;
	ret.fromString(ipgw);
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
    	return WiFiInterface()->get_rssi();
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

	uint8_t attempts = sizeof(aps) / sizeof(aps[0]);

 	int8_t count = WiFiInterface()->scan(aps, attempts);
	 if(count > 0)
	 {
		 current_status = WL_SCAN_COMPLETED;
	 }
	 return count;
}


const char* WiFiClass::SSID(uint8_t networkItem)
{
	if (networkItem >= ap_number)
	{
		return NULL;
	}
	
	return aps[networkItem]->get_ssid();
}

int32_t WiFiClass::RSSI(uint8_t networkItem)
{
	if (networkItem >= ap_number)
	{
		return NULL;
	}
	
	return aps[networkItem]->get_rssi();
}


uint8_t WiFiClass::encryptionType(uint8_t networkItem)
{
	if (networkItem >= ap_number)
	{
		return ENC_TYPE_NONE;
	}

	nsapi_security_t ret = aps[networkItem]->get_security();
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

