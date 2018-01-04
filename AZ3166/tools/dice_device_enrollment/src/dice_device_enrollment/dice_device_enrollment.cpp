// dice_device_enrollment.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "DiceCore.h"
#include "DiceRIoT.h"

// User inputs
char *udsString;
char *registrationId;
char *macAddress;
char *firmwareVer;

// Input file names
const char *binFileFullPath = ".\\DPS.ino.bin";
const char *mapFileFullPath = ".\\DPS.ino.map";

// Settings
uint32_t udsStringLength = 64;
uint32_t registrationIdMaxLength = 128;
uint32_t autoGenRegistrationIdMaxLength = 32;
uint32_t macAddressLength = 12;
uint32_t elementSize = 1; //bytes
uint32_t aliasCertSize = 1024; //bytes

							   // Flash start address name
const char * flashStartname = "FLASH            0x";

// Riot attribute names
const char * startRiotCoreName = "__start_riot_core = .";
const char * stopRiotCoreName = "__stop_riot_core = .";
const char * startRiotFwName = "__start_riot_fw = .";
const char * stopRiotFwName = "__stop_riot_fw = .";

// Flash start info
uint8_t * startBin;

// Riot Core Info
uint8_t * startRiotCore;
uint8_t * endRiotCore;

// Riot Firmware Info
uint8_t * startRiotFw;
uint8_t * endRiotFw;

// UDS bytes for DICE|RIoT calculation
uint8_t udsBytes[DICE_UDS_LENGTH] = { 0 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get user input
static char* get_user_input(const char* text_value, int max_len)
{
	char* result = (char*)malloc(max_len + 1);
	if (result == NULL)
	{
		(void)printf("failed to allocate buffer\r\n");
		return result;
	}
	else
	{
		int index = 0;
		memset(result, 0, max_len + 1);
		printf("%s", text_value);
		// trim the leading spaces
		while (1)
		{
			int c = getchar();
			if (c == EOF || c == 0xA)
				break;
			if (!isspace(c))
			{
				ungetc(c, stdin);
				break;
			}
		}

		while (1)
		{
			int input = getchar();
			if (isspace(input) || input == EOF || input == 0xA)
			{
				break;
			}
			result[index++] = (char)input;
			if (index == max_len)
			{
				// Will need to clear out the remaining buffer
				while (input != EOF && input != 0xA)
				{
					input = getchar();
				}
				break;
			}
		}
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Validate user input data sanity
static int udsStringValidated()
{
	// Check length
	int length = strlen(udsString);

	if (length == 0)
	{
		printf("udsString can NOT be NULL.\r\n");
		return 1;
	}

	if (length != udsStringLength)
	{
		printf("udsString must be %d in length but your input length is %d.\r\n", udsStringLength, length);
		return 1;
	}

	// Check each character
	int tempRes = 0;
	char element[2];
	memset(element, 0, 2);
	int badCharCount = 0;
	for (int i = 0; i < udsStringLength; i++)
	{
		element[0] = udsString[i];
		tempRes = strtoul(element, NULL, 16);
		if (tempRes == 0 && element[0] != '0')
		{
			badCharCount++;
			printf("udsString can only contain characters in Hex value (between 0 and e) and the %dth character in your string is %s.\r\n", i + 1, element);
		}
	}

	if (badCharCount > 0)
	{
		return 1;
	}
	return 0;
}

static int registrationIdValidated()
{
	// Check length
	int length = strlen(registrationId);
	if (length > registrationIdMaxLength)
	{
		printf("registrationId must be shorter than %d in length but your input length is %d.\r\n", registrationIdMaxLength, length);
		return 1;
	}

	// Check each character
	for (int i = 0; i < length; i++)
	{
		if ((registrationId[i] >= 'a' && registrationId[i] <= 'z')
			|| (registrationId[i] >= '0' && registrationId[i] <= '9')
			|| (registrationId[i] == '-')
			) {
			continue;
		}
		else {
			printf("Input value for registrationId in DPS.ino does not meet DPS requirements - only alphanumeric, lowercase, and hyphen are supported.\r\n");
			return 1;
		}
	}

	return 0;
}

static int macAddressValidated()
{
	// Check length
	int length = strlen(macAddress);
	if (length != macAddressLength)
	{
		printf("macAddress must be %d in length but your input length is %d.\r\n", macAddressLength, length);
		return 1;
	}
	return 0;
}

static int firmwareVerValidated()
{
	int length = strlen(firmwareVer);
	if (length == 0)
	{
		printf("firmwareVer can NOT be NULL.\r\n");
		return 1;
	}

	// Check format
	int dotCount = 0;

	int tempRes = 0;
	char element[2];
	memset(element, 0, 2);
	int badCharCount = 0;

	for (int i = 0; i < length; i++)
	{
		element[0] = firmwareVer[i];
		if (element[0] == '.') {
			dotCount++;
		}
		else
		{
			tempRes = strtoul(element, NULL, 10);
			if (tempRes == 0 && element[0] != '0')
			{
				badCharCount++;
				printf("firmwareVer must contain characters between dots but the %dth character in your input is %s.\r\n", i + 1, element);
			}
		}
	}
	if (dotCount != 2)
	{
		printf("firmwareVer must be num.num.num in format but your input is in wrong format.\r\n");
	}
	if (badCharCount > 0)
	{
		return 1;
	}

	return 0;
}

static int fileFullPathValidated(const char * fileFullPath)
{
	FILE *fp;
	fp = fopen(fileFullPath, "rb"); // open the file in binary format
	if (fp == NULL) {
		printf("Unable to open file %s.\r\n", fileFullPath);
		return 1;
	}

	fclose(fp);
	return 0;
}

static int validateInputFiles()
{
	if (fileFullPathValidated(binFileFullPath) || fileFullPathValidated(mapFileFullPath))
	{
		return 1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prepare DICE|RIOT data
static int getRegistrationId()
{
	for (int i = 0; i < strlen(firmwareVer); i++) {
		if (firmwareVer[i] == '.') {
			firmwareVer[i] = 'v';
		}
	}

	//registrationId = (char*)malloc(autoGenRegistrationIdMaxLength);
	//memset(registrationId, 0, autoGenRegistrationIdMaxLength);
	return sprintf(registrationId, "az-%sv%s", macAddress, firmwareVer);
}

static int getUDSBytesFromString()
{
	char element[2];
	unsigned int resLeft;
	unsigned int resRight;

	memset(element, 0, 2);
	for (int i = 0; i < 32; i++) {
		element[0] = udsString[i * 2];
		resLeft = strtoul(element, NULL, 16);
		element[0] = udsString[i * 2 + 1];
		resRight = strtoul(element, NULL, 16);
		udsBytes[i] = (resLeft << 4) + resRight;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read machine code from binary file
static int getDataFromBinFile(uint8_t * startAddress, uint8_t * buffer, uint32_t size)
{
	int result = 0;
	// Open binary file
	FILE *fp;
	fp = fopen(binFileFullPath, "rb"); // open the file in binary format
	if (fp == NULL) {
		printf("Unable to open file\r\n");
		result = 1;
		return result;
	}

	long int offset = startAddress - startBin;
	result = fseek(fp, offset, SEEK_SET);
	if (result != 0)
	{
		printf("Failed to seek offset %d for binary file %s.\r\n", offset, binFileFullPath);
		result = 1;
		return result;
	}

	uint8_t * curser = buffer;
	for (int i = 0; i < size; i++) {
		result = fread(curser, elementSize, 1, fp);
		if (result != elementSize)
		{
			printf("Failed to read data in size %d bytes from binary file %s.\r\n", elementSize, binFileFullPath);
			result = 1;
			return result;
		}
		curser += elementSize;
	}

	// Close binary file
	fclose(fp);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read RIOT attribute data from map file
static unsigned long int findAddressInMapFile(const char * attributeName)
{
	unsigned long int result = 0;
	FILE *fpm;
	char str[64];

	/* opening file for reading */
	fpm = fopen(mapFileFullPath, "r");
	if (fpm == NULL) {
		perror("Error opening file");
		return(-1);
	}

	while ((fgets(str, 64, fpm)) != NULL) {
		if ((strstr(str, attributeName)) != NULL) {
#if logging
			printf("A match found on line: %s\r\n", str);
#endif
			int pos = 0;
			for (int i = 0; i < 64; i++) {
				if (str[i] == 'x') {
					pos = i + 1;
					break;
				}
			}

			unsigned long int tempInt = 0;
			char temp[2];
			memset(temp, 0, 2);
			for (int i = 0; i < 8; i++) {
				temp[0] = str[pos + i];
				tempInt = strtoul(temp, NULL, 16);
				result = result + (tempInt << ((8 - i - 1) * 4));
			}
#if logging
			printf("%x", result);
#endif
		}
	}

	fclose(fpm);

	if (result == 0)
	{
		printf("No matching for keyword %s is found in file %s.\r\n", attributeName, mapFileFullPath);
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// App start
int main()
{
	int result = 0;

	// Check sanity of input files - .bin and .map
	if (validateInputFiles() != 0) {
		result = 1;
		goto EXIT;
	}

	// Get user input
	udsString = get_user_input("Input the UDS you saved into security chip of your DevKit: ", 64);
	if (udsStringValidated() != 0) {
		result = 1;
		goto EXIT;
	}
	else {
		// Prepare UDS from udsString
		getUDSBytesFromString();
	}

	registrationId = get_user_input("Input your preferred registrationId as you input in DPS.ino: ", 128);
	if (strlen(registrationId) == 0) {
		macAddress = get_user_input("Input the Mac Address on your DevKit: ", 12);
		if (macAddressValidated() != 0) {
			result = 1;
			goto EXIT;
		}
		firmwareVer = get_user_input("Input the firmware version of the program running on your DevKit: ", 5);
		if (firmwareVerValidated() != 0) {
			result = 1;
			goto EXIT;
		}

		// Get desired firmware version and device's registration Id
		//char autoGenRegistrationId[32] = { '\0' };
		getRegistrationId();
	}
	else {
		if (registrationIdValidated()) {
			result = 1;
			goto EXIT;
		}
	}

	// Get start address of flash from .bin file
	unsigned long int resultAddress;
	resultAddress = findAddressInMapFile(flashStartname);
	if (resultAddress == 0)
	{
		result = 1;
		goto EXIT;
	}
	startBin = (uint8_t*)resultAddress;

	// Initilize the value of riot attributes stuff
	resultAddress = findAddressInMapFile(startRiotCoreName);
	if (resultAddress == 0)
	{
		result = 1;
		goto EXIT;
	}
	startRiotCore = (uint8_t*)resultAddress;

	resultAddress = findAddressInMapFile(stopRiotCoreName);
	if (resultAddress == 0)
	{
		result = 1;
		goto EXIT;
	}
	endRiotCore = (uint8_t*)resultAddress;

	resultAddress = findAddressInMapFile(startRiotFwName);
	if (resultAddress == 0)
	{
		result = 1;
		goto EXIT;
	}
	startRiotFw = (uint8_t*)resultAddress;

	resultAddress = findAddressInMapFile(stopRiotFwName);
	if (resultAddress == 0)
	{
		result = 1;
		goto EXIT;
	}
	endRiotFw = (uint8_t*)resultAddress;

	// Read Riot_core machine code from .bin file
	uint32_t riotSize = endRiotCore - startRiotCore;
	uint8_t *riotCore = (uint8_t*)malloc(riotSize);
	memset(riotCore, 0, riotSize);

	if (getDataFromBinFile(startRiotCore, riotCore, riotSize) != 0) {
		free(riotCore);
		result = 1;
		goto EXIT;
	}

	// Read Riot_fw machine code from .bin file
	uint32_t riotFwSize = endRiotFw - startRiotFw;
	uint8_t * riotFw = (uint8_t*)malloc(riotFwSize);
	memset(riotFw, 0, riotFwSize);

	if (getDataFromBinFile(startRiotFw, riotFw, riotFwSize) != 0) {
		free(riotFw);
		result = 1;
		goto EXIT;
	}

	// Retrieve alias certificate
	char* aliasCertBuffer = (char*)malloc(aliasCertSize);
	memset(aliasCertBuffer, 0, aliasCertSize);
	DiceRIoTStart(registrationId, riotCore, riotSize, riotFw, riotFwSize, aliasCertBuffer, aliasCertSize);

	// Write the cert to pem file
	FILE * opening;
	char certFileName[64] = { '\0' };
	sprintf(certFileName, "%s.pem", registrationId);
	opening = fopen(certFileName, "w");
	fprintf(opening, aliasCertBuffer);
	fclose(opening);
	printf("Writing to the Alias Key Certificate file %s is successful.\n", certFileName);

	// Release allocations in heap
	free(riotCore);
	free(riotFw);
	free(aliasCertBuffer);

EXIT:
	(void)printf("Press any key to continue:\r\n");
	(void)getchar();
	return result;
}
