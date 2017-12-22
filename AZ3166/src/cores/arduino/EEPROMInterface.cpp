// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "mbed.h"
#include "HAL_STSAFE-A100.h"
#include "stm32f4xx_hal.h"
#include "EEPROMInterface.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define PCROP_ADDR 0x08008000  // Sector 2 STM32F412
#define PCROP_ENABLED_VALUE  0x8000 /* SPRMOD bit */
#define I2C_ADRESS 0x20
#define OK 0
const static int ZONE_SIZE[11] = {976, 0, 192, 120, 0, 584, 680, 784, 880, 0, 88};

EEPROMInterface::EEPROMInterface()
{
}

EEPROMInterface::~EEPROMInterface()
{
	Free_HAL(handle);
}

void EEPROMInterface::enableHostSecureChannel()
{
	// Initialization of global variable will fail if Init_HAL called in construction function
	Init_HAL(I2C_ADRESS, &handle);
	if (isHostSecureChannelEnabled())
	{
		return;
	}
	Init_Perso(handle, 0, 0, NULL);
	Free_HAL(handle);
	Init_HAL(I2C_ADRESS, &handle);
	uint8_t buf[1000];
	for (int zone = 0; zone < 11; ++zone)
	{
		if (ZONE_SIZE[zone] == 0) continue;
		readWithoutEnvelope(buf, ZONE_SIZE[zone], 0, zone);
		for (int i = 0; i * 480 < ZONE_SIZE[zone]; i ++)
		{
			if (HAL_Store_Data_WithinEnvelop(handle, zone, min(ZONE_SIZE[zone] - i * 480, 480), buf + i * 480, i * 488))
			{
				return;
			}
		}
	}
}

// return -1 on fail, return 0 on success
int EEPROMInterface::write(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
	Init_HAL(I2C_ADRESS, &handle);
	if (dataBuff == NULL || checkZoneSize(dataZoneIndex, buffSize, true) != OK)
    {
        return -1;
    }
	if (isHostSecureChannelEnabled())
	{
		return writeWithEnvelope(dataBuff, buffSize, dataZoneIndex);
	}
	else
	{
		return writeWithoutEnvelope(dataBuff, buffSize, dataZoneIndex);
	}
}

// return -1 on fail, return size otherwise
int EEPROMInterface::read(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)
{
	Init_HAL(I2C_ADRESS, &handle);
	if (dataBuff == NULL || buffSize <= 0)
    {
        return -1;
    }
	int size = buffSize + offset;
	if (checkZoneSize(dataZoneIndex, size, false) != OK || size <= offset)
	{
		return -1;
	}
	if (isHostSecureChannelEnabled())
	{
		printf("secure read!\r\n");
		return readWithEnvelope(dataBuff, size - offset, offset, dataZoneIndex);
	}
	else
	{
		printf("unsecure read!\r\n");
		return readWithoutEnvelope(dataBuff, size - offset, offset, dataZoneIndex);
	}
}

int EEPROMInterface::writeWithEnvelope(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
	int envelopeSize = min(ZONE_SIZE[dataZoneIndex], ((buffSize - 1) / 480 + 1) * 480);
	uint8_t* buf = (uint8_t*)malloc(envelopeSize + 1);
	int readSize = readWithEnvelope(buf, envelopeSize, 0, dataZoneIndex);
	if (readSize != envelopeSize)
	{
		delete buf;
		return -1;
	}
	memcpy(buf, dataBuff, buffSize);
	for (int i = 0; i * 480 < envelopeSize; i ++)
	{
		if (HAL_Store_Data_WithinEnvelop(handle, dataZoneIndex, min(envelopeSize - i * 480, 480), buf + i * 480, i * 488))
		{
			delete buf;
			return -1;
		}
	}
	delete buf;
	return 0;
}

int EEPROMInterface::writeWithoutEnvelope(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
	if (HAL_Store_Data_Zone(handle, dataZoneIndex, buffSize, dataBuff, 0x0)) return -1;
	return 0;
}

int EEPROMInterface::readWithEnvelope(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)
{
	int envelopeSize = min(ZONE_SIZE[dataZoneIndex], ((buffSize + offset - 1) / 480 + 1) * 480);
	uint8_t* buf = (uint8_t*)malloc(envelopeSize + 1);
	for (int i = 0; i * 480 < envelopeSize; i ++)
	{
		int readSize = min(envelopeSize - i * 480, 480);
		if (HAL_Get_Data_WithinEnvelop(handle, dataZoneIndex, readSize, buf + i * 480, i * 488))
		{
			memset(buf, 0, readSize);
			HAL_Store_Data_WithinEnvelop(handle, dataZoneIndex, readSize, buf, i * 488);
			delete buf;
			return -1;
		}
	}
	memcpy(dataBuff, buf + offset, buffSize);
	delete buf;
	return buffSize;
}

int EEPROMInterface::readWithoutEnvelope(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)
{
	if (HAL_Get_Data_Zone(handle, dataZoneIndex, buffSize, dataBuff, offset)) return -1;
	return buffSize;
}

bool EEPROMInterface::isHostSecureChannelEnabled()
{
	unsigned short *PCROP_Code_buff = (unsigned short *)PCROP_ADDR;
	if (!PCROPCheck(OB_PCROP_SECTOR_2) && PCROP_Code_buff[0] != 0xb4f0)
	{
		return false;
	}
	return true;
}

bool EEPROMInterface::PCROPCheck(int sector)
{
	FLASH_AdvOBProgramInitTypeDef pAdvOBInit; 
	__IO uint32_t SectorsPCROPStatus = 0x00000000;
	__IO uint16_t PCROPStatus = 0x0000;
	
	/* Get FLASH_PCROP_SECTORS protection status */
	HAL_FLASHEx_AdvOBGetConfig(&pAdvOBInit);
	SectorsPCROPStatus = pAdvOBInit.Sectors & sector;
	PCROPStatus = pAdvOBInit.Sectors & PCROP_ENABLED_VALUE;

	/* Check if sector 2 has been already PCROP-ed */
	if ((PCROPStatus == PCROP_ENABLED_VALUE) && (SectorsPCROPStatus == sector))
		return true;
	else 
		return false;
}

bool EEPROMInterface::checkZoneSize(int dataZoneIndex, int &size, bool write)
{
	if (dataZoneIndex < 0 || dataZoneIndex > 10 || ZONE_SIZE[dataZoneIndex] == 0)
	{
		return 1;
	}
	if (size <= 0)
	{
		return 1;
	}
	int zoneSize = ZONE_SIZE[dataZoneIndex];
	if (write == 1 && size > zoneSize)
	{
		return 1;
	}
	if (size > zoneSize)
	{
		size = zoneSize;
	}
	return OK;
}
