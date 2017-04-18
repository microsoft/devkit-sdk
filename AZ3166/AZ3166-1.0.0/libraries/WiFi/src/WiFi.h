/*
  WiFi.h - Library for Arduino Wifi shield.
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

#ifndef AzureBoard_WiFi_h
#define AzureBoard_WiFi_h

#include <inttypes.h>

extern "C" {
    #include "utility\wl_definitions.h"
}

#include "IPAddress.h"
#include "WiFiClient.h"
//#include "WiFiServer.h"


class WiFiClass
{
public:
    static int16_t  _state[MAX_SOCK_NUM];

    WiFiClass();
    
    /*
     * Get firmware version
     */
    const char* firmwareVersion();

    /* Start Wifi connection with passphrase
     * the SSID and passphrase are from EEPROM.
     */
    int begin(void);

    /* Start Wifi connection with passphrase
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     */
    int begin(char* ssid);

    /* Start Wifi connection with passphrase
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     */
    int begin(char* ssid, const char *passphrase);


    /* Start Wifi connection with access point mode
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     */
    int beginAP(char* ssid, const char *passphrase);

    /*
     * Disconnect from the network
     *
     * return: one value of wl_status_t enum
     */
    int disconnect(void);

    /*
     * Disconnect AP from the network
     *
     * return: one value of wl_status_t enum
     */
    int disconnectAP(void);

    /*
     * Get the interface MAC address.
     *
     * return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
     */
    uint8_t* macAddress(uint8_t* mac);
     
    /*
     * Get the interface IP address.
     *
     * return: Ip address value
     */
    IPAddress localIP();
     
    /*
     * Get the interface subnet mask address.
     *
     * return: subnet mask address value
     */
    IPAddress subnetMask();
     
    /*
     * Get the gateway ip address.
     *
     * return: gateway ip address value
     */
    IPAddress gatewayIP();
     
    /*
     * Return the current SSID associated with the network
     *
     * return: ssid string
     */
    const char* SSID();
     
    /*
     * Return the current BSSID associated with the network.
     * It is the MAC address of the Access Point
     *
     * return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
     */
    uint8_t* BSSID(uint8_t* bssid);
     
    /*
     * Return the current RSSI /Received Signal Strength in dBm)
     * associated with the network
     *
     * return: signed value
     */
    int32_t RSSI();
     
    /*
     * Return the Encryption Type associated with the network
     *
     * return: one value of wl_enc_type enum
     */
    uint8_t encryptionType();
     
    /*
     * Start scan WiFi networks available
     *
     * return: Number of discovered networks
     */
    int8_t scanNetworks();
     
    /*
     * Return the SSID discovered during the network scan.
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: ssid string of the specified item on the networks scanned list
     */
    const char* SSID(uint8_t networkItem);
     
    /*
     * Return the encryption type of the networks discovered during the scanNetworks
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
     */
    uint8_t encryptionType(uint8_t networkItem);
     
    /*
     * Return the RSSI of the networks discovered during the scanNetworks
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: signed value of RSSI of the specified item on the networks scanned list
     */
    int32_t RSSI(uint8_t networkItem);
     
    /*
     * Return Connection status.
     *
     * return: one of the value defined in wl_status_t
     */
    uint8_t status();

    friend class WiFiClient;
    //friend class WiFiServer;

private:
    void MacStringToBin(const char* macString, uint8_t* macBin);

    char firmware_version[128];
    bool is_station_inited;
    bool is_ap_inited;
    char ssid[33];

	uint8_t current_status;

    WiFiAccessPoint aps[10];
	uint8_t ap_number;
};

extern WiFiClass WiFi;

#endif
