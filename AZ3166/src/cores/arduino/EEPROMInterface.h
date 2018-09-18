// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef _EEPROM_INTERFACE_
#define _EEPROM_INTERFACE_

#include "mbed.h"

#define DPS_UDS_ZONE_IDX        0X06
#define WIFI_SSID_ZONE_IDX      0x03
#define WIFI_PWD_ZONE_IDX       0X0A
#define AZ_IOT_HUB_ZONE_IDX     0X05

#define WIFI_SSID_MAX_LEN       32
#define WIFI_PWD_MAX_LEN        64
#define AZ_IOT_HUB_MAX_LEN      512
#define DPS_UDS_MAX_LEN         64
#define EEPROM_DEFAULT_LEN      200

/**
 * \brief Write/Read data to/from EEPROM of STSAFE_A100 through I2C interface.
 *
 */
class EEPROMInterface
{
public:
	EEPROMInterface();
	~EEPROMInterface();

	/**
	* @brief    Write data to secure chip.
	*
	* @param    dataBuff            The data to be written secure chip.
	* @param    buffSize            The size of written data. The valid range of different data zone is different.
	* @param    dataZoneIndex       The index of zone written data to. The valid input is {0, 2, 3, 5, 6, 7, 8, 10}.
	*                               {3, 5, 10} are used for wifi and iot hub connection string. {0, 2, 6} are reserved for later mini solutions.
	*                               So we recommend user to use {7, 8}
	*
	* @return   Return 0 on success, otherwise return -1. The failure might be caused by input dataSize bigger than data zone could write.
	*/
	int write(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex);

	/**
	* @brief    Read data from secure chip.
	*
	* @param    dataBuff            The buffer to store data read from secure chip.
	* @param    buffSize            The size of data need to be read.
	* @param    offset              The offset of data in data zone to start read data from.
	* @param    dataZoneIndex       The index of zone to read data from. The valid input is {0, 2, 3, 5, 6, 7, 8, 10}.
	*                               {3, 5, 10} are used for wifi and iot hub connection string. {0, 2, 6} are reserved for later mini solutions.
	*                               So we recommend user to use {7, 8}
	*
	* @return   Return read buffer size on success, otherwise return -1.
	*/
	int read(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex);

	/**
	* @brief    Enable secure channel between AZ3166 and secure chip.
	*
	* On function called, a key will be set to secure chip, meanwhile the key will be stored in flash of AZ3166.
	* And this will automatically encrypt all data in secure chip.
	* Notice that do not drag-drop firmware after secure channel enabled.
	* Drag-drop bin file may rewrite the flash with key and the data in zone can not be decrypted correctly.
	*
	* @param    level               Secure level of secure channel. 1 means hard code key, 2 means user defined key, 3 means random key.
	*                               The key can never be changed after set. Be very careful here. And we strongly suggest developer use 1 here.
	*                               For now, we only support level 1.
	* @param    key                 A 32 bytes array needed if choose level 2, user defined key.
	*
	* @return   Return 0 on success. Return 1 if the secure channel has already been enabled. Return -1 on fail.
	*           The failure might be caused by wrong parameter or key not available.
	*/
	int enableHostSecureChannel(int level = 1, uint8_t* key = NULL);
private:
	void* handle;
	bool PCROPCheck(int sector);
	bool checkZoneSize(int dataZoneIndex, int &size, bool write);
	bool isHostSecureChannelEnabled();
	int writeWithEnvelope(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex);
	int writeWithoutEnvelope(uint8_t* dataBuff, int buffSize, uint8_t dataZoneIndex);
	int readWithEnvelope(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex);
	int readWithoutEnvelope(uint8_t* dataBuff, int buffSize, uint16_t offset, uint8_t dataZoneIndex);
};

#endif /* _EEPROM_INTERFACE_ */
