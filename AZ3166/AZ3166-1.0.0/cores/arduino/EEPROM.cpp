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
#include "STSAFE_A.h"
#include "STSAFE_A_I2C.h"

#define STSAFE_A_POWER_PIN PB_14

EEPROMClass::EEPROMClass()
{
    
}

EEPROMClass::~EEPROMClass()
{

}

uint8_t EEPROMClass::read(int idx)
{
    uint8_t zoneIndex = (idx & 0x7F000000) >> 24;
    uint16_t dataOffset = idx & 0x3FF;

    int maxBuffLength = getMaxLengthInZone(zoneIndex);

    if (dataOffset < maxBuffLength)
    {
        uint8_t outData;
        int responseCode = eepromRead(&outData, 1, dataOffset, zoneIndex);

        if (responseCode > 0)
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

    if (buffSize <= maxBuffLength)
    {
        uint8_t* outData = (uint8_t*)malloc(buffSize);
        int result = eepromRead(outData, buffSize, 0, zoneIndex);

        if (result == buffSize)
        {
            memcpy(dataBuff, outData, buffSize);
            free(outData);
            return result;
        }
    }
    
    return 0;
}

int EEPROMClass::eepromRead(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)
{
    if (dataBuff == NULL)
    {
        return -1;
    }
    
    // Power on supply
    DigitalOut STSAFE_A_Power(STSAFE_A_POWER_PIN);
    STSAFE_A_Power = 1;
    DELAY(50);

    // Set up I2C
	cSTSAFE_A_I2c STSAFE_A_I2c(I2C_SDA, I2C_SCL);
	STSAFE_A_I2c.Frequency(400000);

    // Configure I2C communication buffer
	cSTSAFE_A Peripheral(&STSAFE_A_I2c, 0x20);
	Peripheral.DataBufferConfiguration();

    // Read
	ReadBuffer* sts_read = NULL;
	ResponseCode result = Peripheral.Read(0x00, 0x00, ALWAYS, dataZoneIndex, offset, buffSize, &sts_read, NO_MAC);
	if (result == 0)
	{
        memcpy(dataBuff, sts_read->Data, sts_read->Length);
        return sts_read->Length;
	}
    
    return -1;
}

int getMaxLengthInZone(uint8_t zoneIndex)
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
