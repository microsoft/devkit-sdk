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
#include "EEPROMInterface.h"

uint8_t EEPROMClass::read(int idx)
{
    uint8_t zoneIndex = (idx & 0x7F000000) >> 24;
    uint16_t dataOffset = idx & 0x3FF;

    int maxBuffLength = getMaxLengthInZone(zoneIndex);

    if (dataOffset < maxBuffLength)
    {
        EEPROMInterface eepromInterface;
        uint8_t outData;
        int result = eepromInterface.read(&outData, 1, dataOffset, zoneIndex);

        if (result > 0)
        {
            return outData;
        }
    }
    
    return 0;
}

int EEPROMClass::getData(int idx, char* dataBuff, int buffSize)
{
    uint8_t zoneIndex = (idx & 0x7F000000) >> 24;
    int maxBuffLength = getMaxLengthInZone(zoneIndex);
    if (buffSize > maxBuffLength)
    {
        return 0;
    }
    
    uint8_t* outData = (uint8_t*)malloc(buffSize);
    if (outData == NULL)
    {
        return 0;
    }
    
    EEPROMInterface eepromInterface;
    int result = eepromInterface.read(outData, buffSize, 0, zoneIndex);
    if (result == buffSize)
    {
        memcpy(dataBuff, outData, buffSize);
    }
    else
    {
        result = 0;
    }
    free(outData);
    
    return result;
}

static int getMaxLengthInZone(uint8_t zoneIndex)
{
    int length = 0;

    switch(zoneIndex)
    {
        case WIFI_SSID_ZONE_IDX:
            length = WIFI_SSID_MAX_LEN;
            break;
        case WIFI_PWD_ZONE_IDX:
            length = WIFI_PWD_MAX_LEN;
            break;
        case AZ_IOT_HUB_ZONE_IDX:
            length = AZ_IOT_HUB_MAX_LEN;
            break;
        default:
            length = EEPROM_DEFAULT_LEN;
            break;
    }

    return length;
}
