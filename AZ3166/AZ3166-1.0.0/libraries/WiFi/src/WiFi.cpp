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

#include "AzureBoardWiFi.h"
#include "utility\wifi_drv.h"
#include "wiring.h"
#include "utility\wl_types.h"

int16_t 	WiFiClass::_state[MAX_SOCK_NUM] = { NA_STATE, NA_STATE, NA_STATE, NA_STATE };

static uint8_t wifi_inited = 0;

WiFiClass::WiFiClass()
{
}

void WiFiClass::init()
{
	if ( wifi_inited == 0 ) {
		wiFiDrv.wifiDriverInit();
		wifi_inited = 1;
	}
}

char* WiFiClass::firmwareVersion()
{
//	return WiFiDrv::getFwVersion();
    return "1.1.0";
}

int WiFiClass::begin(char* ssid)
{
	WiFiClass::init();
	this->begin(ssid, NULL);
}

int WiFiClass::begin(char* ssid, const char *passphrase)
{
	int8_t ret;

	WiFiClass::init();
	
	if (passphrase == NULL )
		ret =  wiFiDrv.wifiSetPassphrase(ssid, strlen(ssid), NULL, 0);
	else
		ret =  wiFiDrv.wifiSetPassphrase(ssid, strlen(ssid), passphrase, strlen(passphrase));

    if ( ret == WL_SUCCESS ) {
		ret = this->status();
	}
	return ret;
}

int WiFiClass::beginAP(char* ssid, const char *passphrase)
{
	int8_t ret;

	WiFiClass::init();	
    ret =  wiFiDrv.wifiSetAccessPoint(ssid, strlen(ssid), passphrase, strlen(passphrase));
    if ( ret == WL_SUCCESS ) {
		ret = this->status();
	}	
	return ret;
}

int WiFiClass::disconnect()
{
    return wiFiDrv.disconnect();
}

int WiFiClass::disconnectAP()
{
    return wiFiDrv.disconnectAP();
}


uint8_t* WiFiClass::macAddress(uint8_t* mac)
{
	uint8_t* _mac = wiFiDrv.getMacAddress();
	for (int i=0; i<WL_MAC_ADDR_LENGTH; i++)
		mac[i] = _mac[WL_MAC_ADDR_LENGTH-i-1];
    return mac;
}

IPAddress WiFiClass::localIP()
{
	IPAddress ret;
	wiFiDrv.getIpAddress(ret);
	return ret;
}

IPAddress WiFiClass::subnetMask()
{
	IPAddress ret;
	wiFiDrv.getSubnetMask(ret);
	return ret;
}

IPAddress WiFiClass::gatewayIP()
{
	IPAddress ret;
	wiFiDrv.getGatewayIP(ret);
	return ret;
}

char* WiFiClass::SSID()
{
    return wiFiDrv.getCurrentSSID();
}

uint8_t* WiFiClass::BSSID(uint8_t* bssid)
{
	uint8_t* _bssid = wiFiDrv.getCurrentBSSID();
	memcpy(bssid, _bssid, WL_MAC_ADDR_LENGTH);
    return bssid;
}

int32_t WiFiClass::RSSI()
{
    return wiFiDrv.get_rssi();
}

uint8_t WiFiClass::encryptionType()
{
    return wiFiDrv.getCurrentEncryptionType();
}

int8_t WiFiClass::scanNetworks()
{
	uint8_t attempts = 10;
	uint8_t numOfNetworks = 0;

	WiFiClass::init();
 	numOfNetworks = wiFiDrv.startScanNetworks();

    if ( numOfNetworks < 0)
		return WL_FAILURE;
	return numOfNetworks;
}


const char* WiFiClass::SSID(uint8_t networkItem)
{
	return wiFiDrv.getSSIDNetoworks(networkItem);
}

int32_t WiFiClass::RSSI(uint8_t networkItem)
{
	return wiFiDrv.getRSSINetoworks(networkItem);
}


uint8_t WiFiClass::encryptionType(uint8_t networkItem)
{
    return wiFiDrv.getEncTypeNetowrks(networkItem);
}


uint8_t WiFiClass::status()
{
    return wiFiDrv.getConnectionStatus();
}

int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult)
{
	return wiFiDrv.getHostByName(aHostname, aResult);
}

WiFiClass WiFi;

