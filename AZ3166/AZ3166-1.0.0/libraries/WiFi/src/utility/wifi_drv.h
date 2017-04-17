/*
  wifi_drv.h - Library for Arduino Wifi shield.
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

#ifndef WiFi_Drv_h
#define WiFi_Drv_h

#include <inttypes.h>

#include "wl_definitions.h"

#include "IPAddress.h"
#include "mico.h"
#include "WifiInterface.h"

class NetworkStack;

class WiFiDrv : public WiFiInterface
{
public:
    /*
     * Driver initialization
     */
    void wifiDriverInit();

    /** Start the interface
     *
     *  Attempts to connect to a WiFi network. Requires ssid and passphrase to be set.
     *  If passphrase is invalid, NSAPI_ERROR_AUTH_ERROR is returned.
     *
     *  @return         0 on success, negative error code on failure
     */
    virtual int connect();

    /** Start the interface
     *
     *  Attempts to connect to a WiFi network.
     *
     *  @param ssid      Name of the network to connect to
     *  @param pass      Security passphrase to connect to the network
     *  @param security  Type of encryption for connection (Default: NSAPI_SECURITY_NONE)
     *  @param channel   This parameter is not supported, setting it to anything else than 0 will result in NSAPI_ERROR_UNSUPPORTED
     *  @return          0 on success, or error code on failure
     */
    virtual int connect(const char *ssid, const char *pass, nsapi_security_t security = NSAPI_SECURITY_NONE,
                                  uint8_t channel = 0 );
    
    /** Set the WiFi network credentials
     *
     *  @param ssid      Name of the network to connect to
     *  @param pass      Security passphrase to connect to the network
     *  @param security  Type of encryption for connection
     *                   (defaults to NSAPI_SECURITY_NONE)
     *  @return          0 on success, or error code on failure
     */
    virtual int set_credentials(const char *ssid, const char *pass, nsapi_security_t security = NSAPI_SECURITY_NONE);


    /** Set the WiFi network channel - NOT SUPPORTED
     *
     * This function is not supported and will return NSAPI_ERROR_UNSUPPORTED
     *
     *  @param channel   Channel on which the connection is to be made, or 0 for any (Default: 0)
     *  @return          Not supported, returns NSAPI_ERROR_UNSUPPORTED
     */
    virtual int set_channel(uint8_t channel);

    /** Stop the interface
     *  @return             0 on success, negative on failure
     */
    virtual int disconnect();

    /** Get the internally stored IP address
     *  @return             IP address of the interface or null if not yet connected
     */
    virtual const char *get_ip_address();

    /** Get the internally stored MAC address
     *  @return             MAC address of the interface
     */
    virtual const char *get_mac_address();

     /** Get the local gateway
     *
     *  @return         Null-terminated representation of the local gateway
     *                  or null if no network mask has been recieved
     */
    virtual const char *get_gateway();

    /** Get the local network mask
     *
     *  @return         Null-terminated representation of the local network mask
     *                  or null if no network mask has been recieved
     */
    virtual const char *get_netmask();

    /** Gets the current radio signal strength for active connection
     *
     * @return          Connection strength in dBm (negative value)
     */
    virtual int8_t get_rssi();

    /** Scan for available networks
     *
     * This function will block.
     *
     * @param  ap       Pointer to allocated array to store discovered AP
     * @param  count    Size of allocated @a res array, or 0 to only count available AP
     * @param  timeout  Timeout in milliseconds; 0 for no timeout (Default: 0)
     * @return          Number of entries in @a, or if @a count was 0 number of available networks, negative on error
     *                  see @a nsapi_error
     */
    virtual int scan(WiFiAccessPoint *res, unsigned count);

    /*
     * Start scan WiFi networks available
     *
     * return: Number of discovered networks
     */
    int8_t startScanNetworks();

	const char* getSSIDNetoworks(uint8_t networkItem);

	int32_t getRSSINetoworks(uint8_t networkItem);
	
	uint8_t getEncTypeNetowrks(uint8_t networkItem);

    /* Start Wifi connection with passphrase
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param ssid_len: Lenght of ssid string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     * param len: Lenght of passphrase string.
     * return: WL_SUCCESS or WL_FAILURE
     */
   int8_t wifiSetPassphrase(char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t passphrase_len);

    /* Start Wifi connection with accesspoint
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param ssid_len: Lenght of ssid string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     * param len: Lenght of passphrase string.
     * return: WL_SUCCESS or WL_FAILURE
     */
   int8_t wifiSetAccessPoint(char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t passphrase_len);

    /*
	* Disconnect AP from the network
	*
	* return: WL_SUCCESS or WL_FAILURE
	*/
   int8_t disconnectAP();

   uint8_t* getMacAddress();

   /*
	* Get the interface IP address.
	*
	* return: copy the ip address value in IPAddress object
	*/
   void getIpAddress(IPAddress& ip);
   
   /*
	* Get the interface subnet mask address.
	*
	* return: copy the subnet mask address value in IPAddress object
	*/
   void getSubnetMask(IPAddress& mask);
   
   /*
	* Get the gateway ip address.
	*
	* return: copy the gateway ip address value in IPAddress object
	*/
   void getGatewayIP(IPAddress& ip);

   /*
	* Return the current SSID associated with the network
	*
	* return: ssid string
	*/
    char* getCurrentSSID();
   
   /*
	* Return the current BSSID associated with the network.
	* It is the MAC address of the Access Point
	*
	* return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
	*/
   uint8_t* getCurrentBSSID();

   /*
	* Return the Encryption Type associated with the network
	*
	* return: one value of wl_enc_type enum
	*/
   uint8_t getCurrentEncryptionType();
   
   
   uint8_t getConnectionStatus();

   
   /*
	* Return Connection status.
	*
	* return: one of the value defined in wl_status_t
	*/
   uint8_t status();

   
   int getHostByName(const char* aHostname, IPAddress& aResult);


     /** Provide access to the underlying stack
     *
     *  @return The underlying network stack
     */
    NetworkStack *get_stack();

private: 

    bool _is_sta_connected;
    bool _is_ap_connected;
    uint8_t wifi_status;
    uint8_t ap_ch;

    char ap_ssid[33]; /* 32 is what 802.11 defines as longest possible name; +1 for the \0 */
    char ap_pass[64]; /* The longest allowed passphrase */

    static void _wlan_status_cb_by_mico( WiFiEvent event, void *inContext );
    void _wlan_status_cb( WiFiEvent event );

    static void _scan_complete_cb_by_mico( ScanResult_adv *pApList, void *inContext );
    void _scan_complete_cb( ScanResult_adv *pApList );
};

extern WiFiDrv wiFiDrv;

#endif
