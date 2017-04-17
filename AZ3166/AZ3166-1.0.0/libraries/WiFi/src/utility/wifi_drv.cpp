/*
  wifi_drv.cpp - Library for Arduino Wifi shield.
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "variant.h"

#include "Arduino.h"
#include "rtos.h"
#include "wl_types.h"
#include "wifi_drv.h"
#include "emw10xx_lwip_stack.h"

static bool _WiFiDrv_inited = false;
static Semaphore _conn_sem;

#define WiFiDrv_INITED   if( _WiFiDrv_inited == false ) { \
                                    mico_board_init(); \
                                    mico_system_init( (mico_Context_t *)mico_system_context_init( 0 ) ); \
                                    mico_system_notify_register( mico_notify_WIFI_STATUS_CHANGED, (void *)(&WiFiDrv::_wlan_status_cb_by_mico), this );\
                                    _WiFiDrv_inited = true;}

// scan
static Semaphore _scan_sem;
static unsigned _scan_cnt;
static WiFiAccessPoint scannedNetworks[WL_NETWORKS_LIST_MAXNUM];


//
static char 	_ssid[WL_SSID_MAX_LENGTH];
static uint8_t	_bssid[WL_MAC_ADDR_LENGTH];
static uint8_t _mac[WL_MAC_ADDR_LENGTH];
static char	_localIp[WL_IPV4_LENGTH];
static char	_subnetMask[WL_IPV4_LENGTH];
static char	_gatewayIp[WL_IPV4_LENGTH];


/* TCP/IP and Network Interface Initialisation */
static char gateway[17] = "\0";
static char networkmask[17] = "\0";
static bool use_dhcp = false;
static bool if_enabled = false;

void WiFiDrv::_wlan_status_cb_by_mico( WiFiEvent event, void *inContext )
{
    WiFiDrv *handler = (WiFiDrv*) inContext;
    handler->_wlan_status_cb( event );
}


void WiFiDrv::_scan_complete_cb_by_mico( ScanResult_adv *pApList, void *inContext )
{
    WiFiDrv *handler = (WiFiDrv*) inContext;
    handler->_scan_complete_cb( pApList );
}

void WiFiDrv::_scan_complete_cb( ScanResult_adv *pApList )
{
    unsigned i = 0;
    nsapi_wifi_ap_t ap;

    if ( _scan_cnt != 0 ) {
        _scan_cnt = (pApList->ApNum > _scan_cnt) ? _scan_cnt : pApList->ApNum;
        for ( i = 0; i < _scan_cnt; i++ ) {
            memset( &ap, 0x0, sizeof(nsapi_wifi_ap_t));
            strncpy( ap.ssid, pApList->ApList[i].ssid, 32);
            memcpy( ap.bssid, pApList->ApList[i].bssid, 6 );
            ap.rssi = pApList->ApList[i].signal_strength;
            ap.channel = pApList->ApList[i].channel;

            switch( pApList->ApList[i].security ) {
                case SECURITY_TYPE_NONE:
                    ap.security = NSAPI_SECURITY_NONE;
                    break;
                case SECURITY_TYPE_WEP:
                    ap.security = NSAPI_SECURITY_WEP;
                    break;
                case SECURITY_TYPE_WPA_TKIP:
                case SECURITY_TYPE_WPA_AES:
                    ap.security = NSAPI_SECURITY_WPA;
                    break;
                case SECURITY_TYPE_WPA2_TKIP:
                case SECURITY_TYPE_WPA2_AES:
                case SECURITY_TYPE_WPA2_MIXED:
                    ap.security = NSAPI_SECURITY_WPA2;
                    break;
            }
            scannedNetworks[i] = WiFiAccessPoint(ap);
        }
    }
    else{
        _scan_cnt = pApList->ApNum;
    }
    _scan_sem.release( );
}



void WiFiDrv::_wlan_status_cb( WiFiEvent event )
{
    DigitalOut ledWiFi(LED_WIFI);
    switch ( event ) {
        case NOTIFY_STATION_UP:
            _conn_sem.release( );
            _is_sta_connected = true;
            ledWiFi = 0;
            break;
        case NOTIFY_STATION_DOWN:
            _is_sta_connected = false;
            ledWiFi = 1;
            break;
        case NOTIFY_AP_UP:
            _conn_sem.release( );
            _is_ap_connected = true;
            ledWiFi = 0;
            break;
        case NOTIFY_AP_DOWN:
            _is_ap_connected = false;
            ledWiFi = 1;
            break;
        default:
            break;
    }
    return;
}


// Public Methods


void WiFiDrv::wifiDriverInit()
{
	_is_sta_connected = false;
    _is_ap_connected = false;
    wifi_status = WL_NO_SSID_AVAIL;
}

int WiFiDrv::disconnect()
{
    WiFiDrv_INITED;
    micoWlanSuspendStation();
    _is_sta_connected = false;
    wifi_status = WL_DISCONNECTED;
    return WL_SUCCESS;
}

int8_t WiFiDrv::disconnectAP()
{
    WiFiDrv_INITED;
    micoWlanSuspendSoftAP();
    _is_ap_connected = false;
    wifi_status = WL_DISCONNECTED;
    return WL_SUCCESS;
}

int8_t WiFiDrv::startScanNetworks()
{
	WiFiDrv_INITED;
    _scan_cnt = WL_NETWORKS_LIST_MAXNUM;

     /* Register user function when wlan scan is completed */
    mico_system_notify_register( mico_notify_WIFI_SCAN_ADV_COMPLETED, (void *)(&WiFiDrv::_scan_complete_cb_by_mico), this );

    micoWlanStartScanAdv();
    _scan_sem.wait();

    //set wifi status
    wifi_status = WL_SCAN_COMPLETED;

    mico_system_notify_remove( mico_notify_WIFI_SCAN_ADV_COMPLETED, (void *)(&WiFiDrv::_scan_complete_cb_by_mico) );
    return _scan_cnt;   
}

int8_t WiFiDrv::wifiSetPassphrase(char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t passphrase_len)
{
    network_InitTypeDef_adv_st  wNetConfigAdv;

    WiFiDrv_INITED;

    /* Initialize wlan parameters */
    memset( &wNetConfigAdv, 0x0, sizeof(wNetConfigAdv) );
    strcpy((char*)wNetConfigAdv.ap_info.ssid, ssid);
    memcpy((char*)wNetConfigAdv.key, passphrase, 64);
    wNetConfigAdv.key_len = passphrase_len>=64 ? 64 : passphrase_len;
    wNetConfigAdv.ap_info.security = SECURITY_TYPE_AUTO;
    wNetConfigAdv.ap_info.channel = ap_ch;
    wNetConfigAdv.dhcpMode = DHCP_Client;
    wNetConfigAdv.wifi_retry_interval = 100;

    /* Connect Now! */
    /* Register user function when wlan scan is completed */
    //mbed_lwip_bringup();
    micoWlanStartAdv(&wNetConfigAdv);

    /* 10 seconds timeout */
    while( 0 != _conn_sem.wait(0) );
    _conn_sem.wait(20*1000);

    if( _is_sta_connected == true )
    {
        wifi_status = WL_CONNECTED;
        return WL_SUCCESS;
    }
    else{
        micoWlanSuspendStation();
        wifi_status = WL_CONNECT_FAILED;
        return WL_FAILURE;
    }
}

int8_t WiFiDrv::wifiSetAccessPoint(char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t passphrase_len)
{
    WiFiDrv_INITED;

    network_InitTypeDef_st wNetConfig;
    /* Initialize wlan parameters */
    memset( &wNetConfig, 0x0, sizeof(wNetConfig) );
    
    int ssidLength = ssid_len >=WL_SSID_MAX_LENGTH ? WL_SSID_MAX_LENGTH : ssid_len;
    strncpy( (char*) wNetConfig.wifi_ssid, ssid, ssidLength );

    int keyLength = passphrase_len >=WL_WPA_KEY_MAX_LENGTH ? WL_WPA_KEY_MAX_LENGTH : passphrase_len;
    strncpy( (char*) wNetConfig.wifi_key, passphrase, keyLength);

    wNetConfig.dhcpMode = DHCP_Server;
    wNetConfig.wifi_mode = Soft_AP;
    wNetConfig.wifi_retry_interval = 100;
    strcpy( (char*) wNetConfig.local_ip_addr, "192.168.0.1" );
    strcpy( (char*) wNetConfig.net_mask, "255.255.255.0" );
    strcpy( (char*) wNetConfig.dnsServer_ip_addr, "192.168.0.1" );
    /* Start Now! */
    micoWlanStart( &wNetConfig );

    /* 10 seconds timeout */
    while ( 0 != _conn_sem.wait( 0 ) );
    _conn_sem.wait( 20 * 1000 );

    if ( _is_ap_connected == true )
    {
        wifi_status = WL_CONNECTED;
        return WL_SUCCESS;
    }
    else {
        micoWlanSuspendSoftAP();
        wifi_status = WL_CONNECT_FAILED;
        return WL_FAILURE;
    }
}



uint8_t* WiFiDrv::getMacAddress()
{
    WiFiDrv_INITED;
    mico_wlan_get_mac_address( _mac );
    return _mac;
}

void WiFiDrv::getIpAddress(IPAddress& ip)
{
	IPStatusTypedef outNetpara;
    if( _is_sta_connected ) 
    {
        micoWlanGetIPStatus(&outNetpara, Station);
        strncpy( _localIp, outNetpara.ip, NSAPI_IPv4_SIZE);
        ip.fromString(_localIp);
    }
}

 void WiFiDrv::getSubnetMask(IPAddress& mask)
 {
    IPStatusTypedef outNetpara;
    if( _is_sta_connected ) 
    {
        micoWlanGetIPStatus(&outNetpara, Station);
        strncpy( _subnetMask, outNetpara.mask, NSAPI_IPv4_SIZE);
        mask.fromString(_subnetMask);
    }
 }

 void WiFiDrv::getGatewayIP(IPAddress& ip)
 {
	IPStatusTypedef outNetpara;
    if( _is_sta_connected ) 
    {
        micoWlanGetIPStatus(&outNetpara, Station);
        strncpy( _gatewayIp, outNetpara.gate, NSAPI_IPv4_SIZE);
        ip.fromString(_gatewayIp);
    }
 }

char* WiFiDrv::getCurrentSSID()
{
	LinkStatusTypeDef link_status;
    if( _is_sta_connected == false )
       return 0;
    micoWlanGetLinkStatus( &link_status );
    memcpy(_ssid, link_status.ssid, WL_SSID_MAX_LENGTH);
    return _ssid;
}

uint8_t* WiFiDrv::getCurrentBSSID()
{
	LinkStatusTypeDef link_status;
    if( _is_sta_connected == false ) return 0;

    micoWlanGetLinkStatus( &link_status );
    memcpy(_bssid, link_status.bssid, WL_MAC_ADDR_LENGTH);
    return _bssid;
}

int8_t WiFiDrv::get_rssi()
{
	LinkStatusTypeDef link_status;
    if( _is_sta_connected == false ) return 0;

    micoWlanGetLinkStatus( &link_status );
    return link_status.signal_strength;
}

uint8_t WiFiDrv::getCurrentEncryptionType()
{
    return NULL;
}


const char* WiFiDrv::getSSIDNetoworks(uint8_t networkItem)
{
	if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
		return NULL;

	return scannedNetworks[networkItem].get_ssid();
}

int32_t WiFiDrv::getRSSINetoworks(uint8_t networkItem)
{
	if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
		return NULL;

	return scannedNetworks[networkItem].get_rssi();
}


uint8_t WiFiDrv::getEncTypeNetowrks(uint8_t networkItem)
{
    if (networkItem >= WL_NETWORKS_LIST_MAXNUM)
	    return NULL;

    nsapi_security_t securityType = scannedNetworks[networkItem].get_security();
    wl_enc_type enc_type;

    switch (securityType)
    {
        case NSAPI_SECURITY_NONE:
            enc_type = ENC_TYPE_NONE;
            break;
        case NSAPI_SECURITY_WEP:
            enc_type = ENC_TYPE_WEP;
            break;
        case NSAPI_SECURITY_WPA:
            enc_type = ENC_TYPE_TKIP;
            break;
        case NSAPI_SECURITY_WPA2:
            enc_type = ENC_TYPE_CCMP;
            break;
        case NSAPI_SECURITY_WPA_WPA2:
            enc_type = ENC_TYPE_AUTO;
            break;        
        case NSAPI_SECURITY_UNKNOWN:
            enc_type = ENC_TYPE_NONE;
            break;
    }
    return enc_type;    
}

uint8_t WiFiDrv::getConnectionStatus()
{
    return wifi_status;
}



int WiFiDrv::getHostByName(const char* aHostname, IPAddress& aResult)
{
	int result = 0;
	return result;
}

NetworkStack* WiFiDrv::get_stack()
{
    WiFiDrv_INITED;
    return nsapi_create_stack(&lwip_stack);
}


int WiFiDrv::set_channel(uint8_t channel)
{
    return NSAPI_ERROR_OK;
}

int WiFiDrv::connect()
{
    int8_t status = wifiSetPassphrase(ap_ssid, strlen(ap_ssid), ap_pass, strlen(ap_pass));
    if(status == WL_SUCCESS)
        return NSAPI_ERROR_OK;
    else
        return NSAPI_ERROR_NO_CONNECTION;
}

int WiFiDrv::connect(const char *ssid, const char *pass, nsapi_security_t security,
                                     uint8_t channel)
{
    if (channel != 0) {
        return NSAPI_ERROR_UNSUPPORTED;
    }

    set_credentials(ssid, pass, security);
    return connect();
}



int WiFiDrv::set_credentials(const char *ssid, const char *pass, nsapi_security_t security)
{
    memset(ap_ssid, 0, sizeof(ap_ssid));
    strncpy(ap_ssid, ssid, sizeof(ap_ssid));

    memset(ap_pass, 0, sizeof(ap_pass));
    strncpy(ap_pass, pass, sizeof(ap_pass));

    return 0;
}

int WiFiDrv::scan(WiFiAccessPoint *res, unsigned count)
{
    int result = startScanNetworks();
    res = scannedNetworks;
    return result;
}



WiFiDrv wiFiDrv;

