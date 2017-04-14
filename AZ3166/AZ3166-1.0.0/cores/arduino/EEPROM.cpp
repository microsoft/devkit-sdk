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

#include "mbed.h"
#include "EEPROM.h"
#include "cli\EEPROMInterface.h"

#define WIFI_MAX_LENGTH 100
#define AZ_CONN_MAX_LENGTH 200
#define DEFAULT_MAX_LENGH 50
#define WIFI_SSID_ZONE_INDEX 0x03
#define WIFI_PWD_ZONE_INDEX 0x0A
#define IOTCONN_ZONE_INDEX 0x05

EEPROMClass::EEPROMClass()
{
}

EEPROMClass::~EEPROMClass()
{
}

uint8_t EEPROMClass::read(int idx)
{
    uint8_t zoneIndex = (idx & 0xF000000) >> 24;
    uint16_t dataOffset = idx & 0x3FF;

    int maxBuffLength = 0;

    switch(zoneIndex)
    {
        case WIFI_SSID_ZONE_INDEX:
            maxBuffLength = WIFI_MAX_LENGTH;
            break;
        case WIFI_PWD_ZONE_INDEX:
            maxBuffLength = WIFI_MAX_LENGTH;
            break;
        case IOTCONN_ZONE_INDEX:
            maxBuffLength = AZ_CONN_MAX_LENGTH;
            break;
        default:
            maxBuffLength = DEFAULT_MAX_LENGH;
            break;
    }

    uint8_t outData[maxBuffLength];
    int responseCode = eepromInterface.read(outData, sizeof(outData), zoneIndex);

    if (responseCode > 0 && responseCode > dataOffset)
    {
        return outData[dataOffset];
    }
    
    return 0;
}
