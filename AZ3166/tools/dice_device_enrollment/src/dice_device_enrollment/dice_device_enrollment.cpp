// dice_device_enrollment.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <ctype.h>
#include <stdbool.h>
#include "DiceCore.h"
#include "DiceRIoT.h"

// Settings
const uint32_t fileNameLength = 256;
const uint32_t udsStringLength = 64;
const uint32_t registrationIdMaxLength = 128;
const uint32_t macAddressLength = 12;
const uint32_t aliasCertSize = 1024; //bytes

// Flash start address name
const char * flashStartname = "FLASH            0x";

// Riot attribute names
const char * startRiotCoreName = "__start_riot_core = .";
const char * stopRiotCoreName = "__stop_riot_core = .";
const char * startRiotFwName = "__start_riot_fw = .";
const char * stopRiotFwName = "__stop_riot_fw = .";

// User inputs
char *fileName;
char *udsString;
char *registrationId;
char *macAddress;
char *firmwareVer;

// Input file names
char binFileFullPath[fileNameLength];
char mapFileFullPath[fileNameLength];


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
        printf("%s", text_value);
        char input[fileNameLength + 2];
        fgets(input, fileNameLength + 2, stdin);
        int index = 0, inputIndex = 0;
        while (inputIndex < strlen(input) && isspace(input[inputIndex])) ++inputIndex;
        while (index < max_len && inputIndex < strlen(input) && isprint(input[inputIndex])) {
            result[index++] = input[inputIndex++];
        }
        result[index] = 0;
        return result;
    }
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

    for (int i = 0; i < length; i++)
    {
        if (firmwareVer[i] == '.') {
            if (i > 0 && firmwareVer[i - 1] == '.')
            {
                printf("firmwareVer must be num.num.num in format but your input is in wrong format.\r\n");
                return 1;
            }
            dotCount++;
        }
        else
        {
            if (firmwareVer[i] > '9' || firmwareVer[i] < '0')
            {
                printf("firmwareVer must contain characters between dots but the %dth character in your input is %c.\r\n", i + 1, firmwareVer[i]);
                return 1;
            }
        }
    }
    if (dotCount != 2 || firmwareVer[0] == '.' || firmwareVer[length - 1] == '.')
    {
        printf("firmwareVer must be num.num.num in format but your input is in wrong format.\r\n");
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
    snprintf(binFileFullPath, fileNameLength, "%s.ino.bin", fileName);
    snprintf(mapFileFullPath, fileNameLength, "%s.ino.map", fileName);
    if (fileFullPathValidated(binFileFullPath) || fileFullPathValidated(mapFileFullPath))
    {
        return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Prepare DICE|RIOT data
static int autoGenRegistrationId()
{
    for (int i = 0; i < strlen(firmwareVer); i++) {
        if (firmwareVer[i] == '.') {
            firmwareVer[i] = 'v';
        }
    }

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
static int getDataFromBinFile(long int offset, uint8_t * buffer, uint32_t size)
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

    result = fseek(fp, offset, SEEK_SET);
    if (result != 0)
    {
        printf("Failed to seek offset %d for binary file %s.\r\n", offset, binFileFullPath);
        result = 1;
        fclose(fp);
        return result;
    }

    uint8_t * curser = buffer;
    result = fread(curser, 1, size, fp);
    if (result != size)
    {
        printf("Failed to read data in size %d bytes from binary file %s.\r\n", size, binFileFullPath);
        fclose(fp);
        return 1;
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
    char str[128];

    /* opening file for reading */
    fpm = fopen(mapFileFullPath, "r");
    if (fpm == NULL) {
        perror("Error opening file");
        return(-1);
    }

    while ((fgets(str, 128, fpm)) != NULL) {
        if ((strstr(str, attributeName)) != NULL && strstr(str, "0x") != NULL) {
#if logging
            printf("A match found on line: %s\r\n", str);
#endif
            char *start = strstr(str, "0x");
            result = strtoul(start, NULL, 16);
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

uint8_t* getBinFileWithName(const char *startName, const char *endName, uint32_t &size)
{
    unsigned long int resultAddress = findAddressInMapFile(startName);
    if (resultAddress == 0)
    {
        return NULL;
    }
    uint8_t* startAddress = (uint8_t*)resultAddress;

    resultAddress = findAddressInMapFile(endName);
    if (resultAddress == 0)
    {
        return NULL;
    }
    uint8_t* endAddress = (uint8_t*)resultAddress;
    size = endAddress - startAddress;
    uint8_t *buf = (uint8_t*)calloc(size, 1);

    if (getDataFromBinFile(startAddress - startBin, buf, size) != 0) {
        free(buf);
        return NULL;
    }
    return buf;
}

int exitFunction(int result){
    (void)printf("Press any key to continue:\r\n");
    (void)getchar();
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// App start
int main()
{
    fileName = get_user_input("Input the name of your project, default name is \"DevKitDPS\" :", fileNameLength);
    if (strlen(fileName) == 0)
    {
        strcpy(fileName, "DevKitDPS");
    }

    // Check sanity of input files - .bin and .map
    if (validateInputFiles() != 0) {
        return exitFunction(1);
    }

    // Get user input
    udsString = get_user_input("Input the UDS you saved into security chip of your DevKit: ", 64);
    if (udsStringValidated() != 0) {
        return exitFunction(1);
    }
    else {
        // Prepare UDS from udsString
        getUDSBytesFromString();
    }

    registrationId = get_user_input("Input your preferred registrationId as you input in DPS.ino(Click Enter to skip if no registrationId provided in DPS.ino): ", 128);
    if (strlen(registrationId) == 0) {
        macAddress = get_user_input("Input the Mac Address on your DevKit: ", 12);
        if (macAddressValidated() != 0) {
            return exitFunction(1);
        }
        firmwareVer = get_user_input("Input the firmware version of the program running on your DevKit: ", 5);
        if (firmwareVerValidated() != 0) {
            return exitFunction(1);
        }

        // Auto Generate registrationId based on firmware version and device MAC address
        autoGenRegistrationId();
    }
    else {
        if (registrationIdValidated()) {
            return exitFunction(1);
        }
    }

    // Get start address of flash from .bin file
    unsigned long int resultAddress;
    resultAddress = findAddressInMapFile(flashStartname);
    if (resultAddress == 0)
    {
        return exitFunction(1);
    }
    startBin = (uint8_t*)resultAddress;

    uint32_t riotSize;
    uint32_t riotFwSize;
    uint8_t* riotCore = getBinFileWithName(startRiotCoreName, stopRiotCoreName, riotSize);
    uint8_t* riotFw = getBinFileWithName(startRiotFwName, stopRiotFwName, riotFwSize);

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

    return exitFunction(0);
}
