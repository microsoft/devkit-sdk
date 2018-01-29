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

#ifndef AZ3166_WiFi_h
#define AZ3166_WiFi_h

extern "C" {
    #include "utility/wl_definitions.h"
}

#include "IPAddress.h"
#include "AZ3166WiFiClient.h"
#include "AZ3166WiFiServer.h"

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
     * return: pointer to unsigned char array with length WL_MAC_ADDR_LENGTH
     */
    unsigned char* macAddress(unsigned char* mac);
     
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
    unsigned char* BSSID(unsigned char* bssid);
     
    /*
     * Return the current RSSI /Received Signal Strength in dBm)
     * associated with the network
     *
     * return: signed value
     */
    int RSSI();
     
    /*
     * Return the Encryption Type associated with the network
     *
     * return: one value of wl_enc_type enum
     */
    int encryptionType();
     
    /*
     * Start scan WiFi networks available
     *
     * return: Number of discovered networks
     */
    int scanNetworks();
     
    /*
     * Return the SSID discovered during the network scan.
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: ssid string of the specified item on the networks scanned list
     */
    const char* SSID(unsigned char networkItem);
     
    /*
     * Return the encryption type of the networks discovered during the scanNetworks
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
     */
    int encryptionType(unsigned char networkItem);
     
    /*
     * Return the RSSI of the networks discovered during the scanNetworks
     *
     * param networkItem: specify from which network item want to get the information
     *
     * return: signed value of RSSI of the specified item on the networks scanned list
     */
    int RSSI(unsigned char networkItem);
     
    /*
     * Return Connection status.
     *
     * return: one of the value defined in wl_status_t
     */
    unsigned char status();

    friend class WiFiClient;
    //friend class WiFiServer;

private:
    void MacStringToBin(const char* macString, unsigned char* macBin);

    char firmware_version[128];
    bool is_wifi_inited;
    bool is_station_inited;
    bool is_ap_inited;
    char ssid[33];

    unsigned char current_status;

    WiFiAccessPoint aps[10];
    int ap_number;
};

extern WiFiClass WiFi;

#endif
