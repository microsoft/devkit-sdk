// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
#include "mbed.h"
#include "HAL_STSAFE-A100.h"
#include "stm32f4xx_hal.h"
#include "EEPROMInterface.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define PCROP_ADDR 0x08008000  // Sector 2 STM32F412
#define PCROP_ENABLED_VALUE  0x8000
#define STSAFE_I2C_ADDRESS 0x20
#define MAX_BUFFER_SIZE 1000
#define MAX_ENCRYPT_DATA_SIZE 480
#define MAX_ENVELOPE_SIZE 488
// The segment length of each data partition
const static int DATA_SEGMENT_LENGTH[11] = {976, 0, 192, 120, 0, 584, 680, 784, 880, 0, 88};

EEPROMInterface::EEPROMInterface()
{
	handle = NULL;
}

EEPROMInterface::~EEPROMInterface()
{
	Free_HAL(handle);
}

void EEPROMInterface::enableHostSecureChannel()
{
	// Initialization of global variable will fail if Init_HAL called in construction function
	Init_HAL(STSAFE_I2C_ADDRESS, &handle);
	if (isHostSecureChannelEnabled())
	{
		return;
	}
	Init_Perso(handle, 0, 0, NULL);
	Free_HAL(handle);
	Init_HAL(STSAFE_I2C_ADDRESS, &handle);
	uint8_t buf[MAX_BUFFER_SIZE];
	for (int dataZoneIndex = 0; dataZoneIndex < 11; ++dataZoneIndex)
	{
		int segmentLength = DATA_SEGMENT_LENGTH[dataZoneIndex];
		if (segmentLength == 0) continue;
		readWithoutEnvelope(buf, segmentLength, 0, dataZoneIndex);
		for (int i = 0; i * MAX_ENCRYPT_DATA_SIZE < segmentLength; i ++)
		{
			int dataSize = min(segmentLength - i * MAX_ENCRYPT_DATA_SIZE, MAX_ENCRYPT_DATA_SIZE);
			if (HAL_Store_Data_WithinEnvelop(handle, dataZoneIndex, dataSize, buf + i * MAX_ENCRYPT_DATA_SIZE, i * MAX_ENVELOPE_SIZE))
			{
				return;
			}
		}
	}
}

// return -1 on fail, return 0 on success
int EEPROMInterface::write(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
	Init_HAL(STSAFE_I2C_ADDRESS, &handle);
	if (dataBuff == NULL || checkZoneSize(dataZoneIndex, buffSize, true))
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
	Init_HAL(STSAFE_I2C_ADDRESS, &handle);
	if (dataBuff == NULL || buffSize <= 0)
    {
        return -1;
    }
	int size = buffSize + offset;
	if (checkZoneSize(dataZoneIndex, size, false) || size <= offset)
	{
		return -1;
	}
	if (isHostSecureChannelEnabled())
	{
		return readWithEnvelope(dataBuff, size - offset, offset, dataZoneIndex);
	}
	else
	{
		return readWithoutEnvelope(dataBuff, size - offset, offset, dataZoneIndex);
	}
}

int EEPROMInterface::writeWithEnvelope(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
	int readSize = min(DATA_SEGMENT_LENGTH[dataZoneIndex], ((buffSize - 1) / MAX_ENCRYPT_DATA_SIZE + 1) * MAX_ENCRYPT_DATA_SIZE);
	uint8_t buf[MAX_BUFFER_SIZE];
	int readResult = readWithEnvelope(buf, readSize, 0, dataZoneIndex);
	if (readResult != readSize)
	{
		return -1;
	}
	memcpy(buf, dataBuff, buffSize);
	for (int i = 0; i * MAX_ENCRYPT_DATA_SIZE < readSize; i ++)
	{
		int dataSize = min(readSize - i * MAX_ENCRYPT_DATA_SIZE, MAX_ENCRYPT_DATA_SIZE);
		if (HAL_Store_Data_WithinEnvelop(handle, dataZoneIndex, dataSize, buf + i * MAX_ENCRYPT_DATA_SIZE, i * MAX_ENVELOPE_SIZE))
		{
			return -1;
		}
	}
	return 0;
}

int EEPROMInterface::writeWithoutEnvelope(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex)
{
	if (HAL_Store_Data_Zone(handle, dataZoneIndex, buffSize, dataBuff, 0x0))
	{
		return -1;
	}
	return 0;
}

int EEPROMInterface::readWithEnvelope(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)
{
	uint8_t buf[MAX_BUFFER_SIZE];
	for (int i = 0; i * MAX_ENCRYPT_DATA_SIZE < buffSize + offset; i ++)
	{
		int dataSize = min(DATA_SEGMENT_LENGTH[dataZoneIndex] - i * MAX_ENCRYPT_DATA_SIZE, MAX_ENCRYPT_DATA_SIZE);
		if (HAL_Get_Data_WithinEnvelop(handle, dataZoneIndex, dataSize, buf + i * MAX_ENCRYPT_DATA_SIZE, i * MAX_ENVELOPE_SIZE))
		{
			memset(buf, 0, dataSize);
			HAL_Store_Data_WithinEnvelop(handle, dataZoneIndex, dataSize, buf, i * MAX_ENVELOPE_SIZE);
			return -1;
		}
	}
	memcpy(dataBuff, buf + offset, buffSize);
	return buffSize;
}

int EEPROMInterface::readWithoutEnvelope(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex)
{
	if (HAL_Get_Data_Zone(handle, dataZoneIndex, buffSize, dataBuff, offset))
	{
		return -1;
	}
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
	if (dataZoneIndex < 0 || dataZoneIndex > 10 || DATA_SEGMENT_LENGTH[dataZoneIndex] == 0)
	{
		return 1;
	}
	if (size <= 0)
	{
		return 1;
	}
	int segmentLength = DATA_SEGMENT_LENGTH[dataZoneIndex];
	if (write == 1 && size > segmentLength)
	{
		return 1;
	}
	if (size > segmentLength)
	{
		size = segmentLength;
	}
	return 0;
}
