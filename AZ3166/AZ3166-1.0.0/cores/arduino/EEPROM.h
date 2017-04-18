/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EEPROM_h
#define EEPROM_h

#include "Arduino.h"

#define WIFI_SSID_ADDRESS （WIFI_SSID_ZONE_IDX << 24)
#define WIFI_PWD_ADDRESS (WIFI_PWD_ZONE_IDX << 24)
#define AZ_IOTCONN_ADDRESS (AZ_IOT_HUB_ZONE_IDX << 24)

#define WIFI_SSID_ZONE_IDX      0x03
#define WIFI_PWD_ZONE_IDX       0X0A
#define AZ_IOT_HUB_ZONE_IDX     0X05

#define WIFI_SSID_MAX_LEN       32
#define WIFI_PWD_MAX_LEN        64
#define AZ_IOT_HUB_MAX_LEN      200
#define EEPROM_DEFAULT_LEN		200

class EEPROMClass
{
  public:
    EEPROMClass();
    ~EEPROMClass();
    uint8_t read(int idx);
    int getData(int idx, char* dataBuff, int buffSize);
  
  private:
    int eepromRead(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex);
};

int getMaxLengthInZone(uint8_t zoneIndex);

static EEPROMClass EEPROM;

#endif