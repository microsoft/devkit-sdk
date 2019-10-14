// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
 
#include "mbed.h"
#include "mico.h"
#include "mbedtls/version.h"
#include "mbedtls/base64.h"
#include "mbedtls/md.h"
#include "mbedtls/md_internal.h"
#include "EEPROMInterface.h"
#include "SystemWiFi.h"
#include "SystemVersion.h"
#include "UARTClass.h"
#include "console_cli.h"
#include "azure_c_shared_utility/connection_string_parser.h"

struct console_command 
{
    const char *name;
    const char *help;
    bool       isPrivacy;
    void (*function) (int argc, char **argv);
};

#define MAX_CMD_ARG         4

#define NULL_CHAR       '\0'
#define RET_CHAR        '\n'
#define END_CHAR        '\r'
#define TAB_CHAR        '\t'
#define SPACE_CHAR      ' '
#define BACKSPACE_CHAR  0x08
#define DEL_CHAR        0x7f
#define PROMPT          "\r\n# "

#define INBUF_SIZE      1024

////////////////////////////////////////////////////////////////////////////////////
// System functions
extern UARTClass Serial;

////////////////////////////////////////////////////////////////////////////////////
// Commands table
static void help_command(int argc, char **argv);
static void get_version_command(int argc, char **argv);
static void reboot_and_exit_command(int argc, char **argv);
static void wifi_scan(int argc, char **argv);
static void wifi_ssid_command(int argc, char **argv);
static void wifi_pwd_Command(int argc, char **argv);
static void az_iothub_command(int argc, char **argv);
static void dps_uds_command(int argc, char **argv);
static void az_iotdps_command(int argc, char **argv);
static void enable_secure_command(int argc, char **argv);

static const struct console_command cmds[] = {
  {"help",          "Help document",                                          false, help_command},
  {"version",       "System version",                                         false, get_version_command},
  {"exit",          "Exit and reboot",                                        false, reboot_and_exit_command},
  {"scan",          "Scan Wi-Fi AP",                                          false, wifi_scan},
  {"set_wifissid",  "Set Wi-Fi SSID",                                         false, wifi_ssid_command},
  {"set_wifipwd",   "Set Wi-Fi password",                                     true,  wifi_pwd_Command},
  {"set_az_iothub", "Set the connection string of Microsoft Azure IoT Hub",   false, az_iothub_command},
  {"set_dps_uds",   "Set DPS Unique Device Secret (DPS)",                     true,  dps_uds_command},
  {"set_az_iotdps", "Format is IdScope=XXX;DeviceId=XXX;SymmetricKey=XXX",    false, az_iotdps_command},
  {"enable_secure", "Enable secure channel between AZ3166 and secure chip",   false, enable_secure_command},
}; 

static const int cmd_count = sizeof(cmds) / sizeof(struct console_command);

static const char *IOTHUBDPS_ENDPOINT = "DPSEndpoint";
static const char *IOTHUBDPS_IDSCOPE = "IdScope";
static const char *IOTHUBDPS_DEVICEID = "DeviceId";
static const char *IOTHUBDPS_SYMMETRICKEY = "SymmetricKey";

//////////////////////////////////////////////////////////////////////////////////////////////
// Generate device SAS token
static int base64DecodeKey(const unsigned char *input, int input_len, unsigned char **output, int *output_len)
{
    unsigned char *buffer = NULL;
    size_t len = 0;
    if (mbedtls_base64_decode(NULL, 0, &len, input, (size_t)input_len) == MBEDTLS_ERR_BASE64_INVALID_CHARACTER)
    {
        return -1;
    }
    buffer = (unsigned char *)calloc(1, len);
    if(buffer == NULL)
    {
        return -2;
    }
    if(mbedtls_base64_decode(buffer, len, &len, input, (size_t)input_len))
    {
        free(buffer);
        return -3;
    }
    *output = buffer;
    *output_len = len;
    return 0;
}

static int base64EncodeKey(const unsigned char *input, int input_len, char **output)
{
    size_t len = 0;
    unsigned char *buffer = NULL;
    mbedtls_base64_encode(NULL, 0, &len, input, (size_t)input_len);
    if (len == 0)
    {
        return -1;
    }
    buffer = (unsigned char *)calloc(1, len + 1);
    if(buffer == NULL)
    {
        return -2;
    }
    if(mbedtls_base64_encode(buffer, len, &len, input, (size_t)input_len))
    {
        free(buffer);
        return -3;
    }
    *output = (char *)buffer;
    return 0;
}

static int GenSasToken(const char *pkey, const char *device_id, char **sas_token)
{
    // Decoode key
    unsigned char *key_data = NULL;
    int key_len = 0;
    if (base64DecodeKey((const unsigned char *)pkey, strlen(pkey), &key_data, &key_len))
    {
        return 1;
    }

    char *token = NULL;
    unsigned char token_data[MBEDTLS_MD_MAX_SIZE] = { 0 };
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    if (mbedtls_md_setup(&ctx, &mbedtls_sha256_info, 1))
    {
        goto _exit;
    }
    if (mbedtls_md_hmac_starts(&ctx, key_data, key_len))
    {
        goto _exit;
    }
    if (mbedtls_md_hmac_update(&ctx, (const unsigned char *)device_id, (size_t)strlen(device_id)))
    {
        goto _exit;
    }
    if (mbedtls_md_hmac_finish(&ctx, token_data))
    {
        goto _exit;
    }
    if (base64EncodeKey(token_data, mbedtls_sha256_info.size, &token))
    {
        goto _exit;
    }
    *sas_token = token;

_exit:
    if (key_data)
    {
        free(key_data);
    }
    mbedtls_md_free(&ctx);
    return (token ? 0 : -1);
}

////////////////////////////////////////////////////////////////////////////////////
// Command handlers
static void print_help()
{
    Serial.print("Configuration console:\r\n");

    for (int i = 0; i < cmd_count; i++)
    {
        Serial.printf(" - %s: %s.\r\n", cmds[i].name, cmds[i].help);
    }
}

static void help_command(int argc, char **argv)
{
    print_help();
}

static void get_version_command(int argc, char **argv)
{
    char ver[128];
    int ret;
    
    Serial.printf( "DevKitSDK version: %s\r\n", getDevkitVersion() );
    Serial.printf( "Mico version: %s\r\n", MicoGetVer() );
    Serial.printf( "mbed-os version: %d.%d.%d\r\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION );
    Serial.printf( "mbed TLS version: %d.%d.%d\r\n", MBEDTLS_VERSION_MAJOR, MBEDTLS_VERSION_MINOR, MBEDTLS_VERSION_PATCH);
        
    memset(ver, 0, sizeof(ver));
    ret = MicoGetRfVer(ver, sizeof(ver));
    if (ret == 0)
    {
        Serial.printf("WIFI version: %s\r\n", ver);
    }
    else
    {
        Serial.printf("WIFI version: unknown\r\n");
    }
}

static void wifi_scan(int argc, char **argv)
{
    WiFiAccessPoint aps[10];
    memset(aps, 0, sizeof(aps));
    int count = WiFiScan(aps, 10);
    if (count > 0)
    {
        Serial.printf("Available networks:\r\n");
        for (int i =0; i < count; i++)
        {
            Serial.printf("  %s\r\n", aps[i].get_ssid());
        }
    }
    else
    {
        Serial.printf("No available network.\r\n");
    }
}

static void reboot_and_exit_command(int argc, char **argv)
{
    Serial.printf("Reboot\r\n");
    mico_system_reboot();
}

static int write_eeprom(char* string, int idxZone)
{    
    EEPROMInterface eeprom;
    int len = strlen(string) + 1;
    
    // Write data to EEPROM
    int result = eeprom.write((uint8_t*)string, len, idxZone);
    if (result != 0)
    {
        Serial.printf("ERROR: Failed to write EEPROM: 0x%02x.\r\n", idxZone);
        return -1;
    }
    
    // Verify
    uint8_t *pBuff = (uint8_t*)malloc(len);
    result = eeprom.read(pBuff, len, 0x00, idxZone);
    if (result != len || strncmp(string, (char*)pBuff, len) != 0)
    {
        Serial.printf("ERROR: Verify failed.\r\n");
        return -1;
    }
    free(pBuff);
    return 0;
}

static void wifi_ssid_command(int argc, char **argv)
{
    if (argc == 1 || argv[1] == NULL) 
    {
        Serial.printf("Usage: set_wifissid <SSID>. Please provide the SSID of the Wi-Fi.\r\n");
        return;
    }
    int len = strlen(argv[1]) + 1;
    if (len == 0 || len > WIFI_SSID_MAX_LEN)
    {
        Serial.printf("Invalid Wi-Fi SSID.\r\n");
        return;
    }
    
    int result = write_eeprom(argv[1], WIFI_SSID_ZONE_IDX);
    if (result == 0)
    {
        Serial.printf("INFO: Set Wi-Fi SSID successfully.\r\n");
    }
}

static void wifi_pwd_Command(int argc, char **argv)
{
    const char *pwd = NULL;
    if (argc == 1)
    {
        // Clean the pwd
        pwd = "";
    }
    else
    {
        if (argv[1] == NULL) 
        {
            Serial.printf("Usage: set_wifipwd [password]. Please provide the password of the Wi-Fi.\r\n");
            return;
        }
        int len = strlen(argv[1]) + 1;
        if (len > WIFI_PWD_MAX_LEN)
        {
            Serial.printf("Invalid Wi-Fi password.\r\n");
        }
        pwd = argv[1];
    }
        
    int result = write_eeprom((char*)pwd, WIFI_PWD_ZONE_IDX);
    if (result == 0)
    {
        Serial.printf("INFO: Set Wi-Fi password successfully.\r\n");
    }
}

static void az_iothub_command(int argc, char **argv)
{
    if (argc == 1 || argv[1] == NULL) 
    {
        Serial.printf("Usage: set_az_iothub <connection string>. Please provide the connection string of the Azure IoT hub.\r\n");
        return;
    }
    int len = strlen(argv[1]) + 1;
    if (len == 0 || len > AZ_IOT_HUB_MAX_LEN)
    {
        Serial.printf("Invalid Azure IoT hub connection string.\r\n");
        return;
    }
    
    int result = write_eeprom(argv[1], AZ_IOT_HUB_ZONE_IDX);
    if (result == 0)
    {
        Serial.printf("INFO: Set Azure Iot hub connection string successfully.\r\n");
    }
}

static void dps_uds_command(int argc, char **argv)
{
    char* uds = NULL;
    if (argc == 1 || argv[1] == NULL)
    {
        Serial.printf("Usage: set_dps_uds [uds]. Please provide the UDS for DPS.\r\n");
        return;
    }

    int len = strlen(argv[1]) + 1;
    if (len != DPS_UDS_MAX_LEN)
    {
        Serial.printf("Invalid UDS.\r\n");
    }
    uds = argv[1];
        
    int result = write_eeprom(uds, DPS_UDS_ZONE_IDX);
    if (result == 0)
    {
        Serial.printf("INFO: Set DPS UDS successfully.\r\n");
    }
}

static void az_iotdps_command(int argc, char **argv)
{
    char *dpsIdScope = NULL;
    char *symmetricKey = NULL;
    char *deviceId = NULL;

    if (argc == 1 || argv[1] == NULL) 
    {
        Serial.printf("Usage: set_az_iotdps <connection string>. Please provide the connection string of the IoT Central.\r\n");
        return;
    }

    MAP_HANDLE connection_string_values_map;
    if ((connection_string_values_map = connectionstringparser_parse_from_char(argv[1])) == NULL)
    {
        Serial.printf("Tokenizing failed on connectionString");
        return;
    }
    const char *_dpsIdScope = Map_GetValueFromKey(connection_string_values_map, IOTHUBDPS_IDSCOPE);
    const char *_symmetricKey = Map_GetValueFromKey(connection_string_values_map, IOTHUBDPS_SYMMETRICKEY);
    const char *_deviceId = Map_GetValueFromKey(connection_string_values_map, IOTHUBDPS_DEVICEID);
    if (_dpsIdScope)
    {
        mallocAndStrcpy_s(&dpsIdScope, _dpsIdScope);
    }
    else
    {
        Serial.printf("Couldn't find %s in connection string", IOTHUBDPS_IDSCOPE);
        return;
    }
    if (_symmetricKey)
    {
        mallocAndStrcpy_s(&symmetricKey, _symmetricKey);
    }
    else
    {
        Serial.printf("Couldn't find %s in connection string", IOTHUBDPS_SYMMETRICKEY);
        return;
    }
    if (_deviceId)
    {
        mallocAndStrcpy_s(&deviceId, _deviceId);
    }
    else
    {
        Serial.printf("Couldn't find %s in connection string", IOTHUBDPS_DEVICEID);
        return;
    }  

    char *sasKey = NULL;
    if (GenSasToken(symmetricKey, deviceId, &sasKey))
    {
        // Failed
        Serial.printf("Couldn't generate SAS Key from %s in connection string", IOTHUBDPS_SYMMETRICKEY);
        return;
    }

    char *dcs = (char*)calloc(AZ_IOT_HUB_MAX_LEN, 1);
    if (dcs == NULL)
    {
        Serial.printf("INFO: Set Iot Central connection string failed. Please try again.\r\n");
        return;
    }
    snprintf(dcs, AZ_IOT_HUB_MAX_LEN, 
             "DPSEndpoint=global.azure-devices-provisioning.net;IdScope=%s;DeviceId=%s;SymmetricKey=%s",
             dpsIdScope,
             deviceId,
             sasKey);
    int len = strlen(dcs) + 1;
    if (len == 0 || len > AZ_IOT_HUB_MAX_LEN)
    {
        Serial.printf("Invalid IoT Central connection string.\r\n");
        free(dcs);
        return;
    }
    int result = write_eeprom(dcs, AZ_IOT_HUB_ZONE_IDX);
    if (result == 0)
    {
        Serial.printf("INFO: Set Iot Central connection string successfully.\r\n");
    }

    free(dcs);
}

static void enable_secure_command(int argc, char **argv)
{
    int ret = -2;
    if (argc > 1 && argv[1] != NULL && strlen(argv[1]) == 1)
    {
        if (argc == 2 && (argv[1][0] == '1' || argv[1][0] == '3'))
        {
            EEPROMInterface eeprom;
            ret = eeprom.enableHostSecureChannel(argv[1][0] - '0');
        }
        else if (argc == 3 && argv[2] != NULL && strlen(argv[2]) == 64)
        {
            int i = 0, val;
            uint8_t key[32];
            char ch;
            memset(key, 0, sizeof(key));
            for (i = 0; i < 64; ++i)
            {
                ch = argv[2][i];
                if (ch <= 'f' && ch >= 'a')
                {
                    val = ch - 'a' + 10;
                }
                else if (ch <= '9' && ch >= '0')
                {
                    val = ch - '0';
                }
                else
                {
                    break;
                }
                key[i / 2] += (i % 2 == 0) ? (val << 4) : val;
            }
            if (i == 64)
            {
                EEPROMInterface eeprom;
                ret = eeprom.enableHostSecureChannel(2, key);
            }
        }
    }

    if (ret == 0)
    {
        Serial.printf("INFO: Enable secure channel successfully.\r\n");
    }
    else if (ret == -1)
    {
        Serial.printf("INFO: Enable secure channel failed.\r\n");
    }
    else if (ret == 1)
    {
        Serial.printf("INFO: Secure channel has already been enabled.\r\n");
    }
    else // enableHostSecureChannel() never run. Input argv does not accepted.
    {
        Serial.printf("Usage: enable_secure <secure level> <provided key>. 64-characters key is only needed for level 2. More detail:\r\n\
        1.\"enable_secure 1\" to enable secure channel with pre set key.\r\n\
        2.\"enable_secure 2 ([a-f]|[0-9]){64}\" to enable secure channel with provided key. (not implemented)\r\n\
        3.\"enable_secure 3\" to enable secure channel with random key. (not implemented)\r\n");
    }
    return;
}
////////////////////////////////////////////////////////////////////////////////////
// Console app
static bool is_privacy_cmd(char *inbuf, unsigned int bp)
{
    // Check privacy mode
    char cmdName[INBUF_SIZE];
    for(unsigned int j = 0; j < bp; j++)
    {
        if (inbuf[j] == SPACE_CHAR)
        {
            // Check the table
            cmdName[j] = 0;
            for(int i = 0; i < cmd_count; i++)
            {
                if(strcmp(cmds[i].name, cmdName) == 0)
                {
                    // It's privacy command
                    return cmds[i].isPrivacy;
                }
            }
            break;
        }
        else
        {
            cmdName[j] = inbuf[j];
        }
    }
    
    return false;
}

static bool get_input(char *inbuf, unsigned int *bp)
{
    if (inbuf == NULL) 
    {
        return false;
    }
    
    while (true) 
    {
        if (!Serial.available())
        {
            continue;
        }
        inbuf[*bp] = (char)Serial.read();
        
        if (inbuf[*bp] == END_CHAR) 
        {
            /* end of input line */
            inbuf[*bp] = NULL_CHAR;
            *bp = 0;
            return true;
        }
        else if (inbuf[*bp] == TAB_CHAR) 
        {
            inbuf[*bp] = SPACE_CHAR;
        }
        else if (inbuf[*bp] == BACKSPACE_CHAR || inbuf[*bp] == DEL_CHAR)
        {
            // Delete
            if (*bp > 0) 
            {
                (*bp)--;
                Serial.write(BACKSPACE_CHAR);
                Serial.write(SPACE_CHAR);
                Serial.write(BACKSPACE_CHAR);
            }
            continue;
        }
        else if (inbuf[*bp] < SPACE_CHAR)
        {
            continue;
        }

        // Echo
        if (!is_privacy_cmd(inbuf, *bp))
        {
            Serial.write(inbuf[*bp]);
        }
        else
        {
            Serial.write('*');
        }
        (*bp)++;
        
        if (*bp >= INBUF_SIZE) 
        {
            Serial.printf("\r\nError: input buffer overflow\r\n");
            Serial.printf(PROMPT);
            *bp = 0;
            break;
        }
    }
    
    return false;
}

static int handle_input(char* inbuf)
{
    struct
    {
        unsigned inArg:1;
        unsigned inQuote:1;
        unsigned done:1;
    } stat;
  
    static char* argv[MAX_CMD_ARG];
    int argc = 0;

    int i = 0;
        
    memset((void*)&argv, 0, sizeof(argv));
    memset(&stat, 0, sizeof(stat));
  
    do 
    {
        switch (inbuf[i]) 
        {
        case '\0':
            if (stat.inQuote)
            {
                return 1;
            }
            stat.done = 1;
            break;
  
        case '"':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) 
            {
                memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
            {
                break;
            }
            if (stat.inQuote && !stat.inArg)
            {
                return 1;
            }
            
            if (!stat.inQuote && !stat.inArg) 
            {
                stat.inArg = 1;
                stat.inQuote = 1;
                argc++;
                argv[argc - 1] = &inbuf[i + 1];
            } 
            else if (stat.inQuote && stat.inArg) 
            {
                stat.inArg = 0;
                stat.inQuote = 0;
                inbuf[i] = '\0';
            }
            break;
      
        case ' ':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) 
            {
                memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                inbuf[i] = '\0';
            }
            break;
        default:
            if (!stat.inArg) 
            {
                stat.inArg = 1;
                argc++;
                argv[argc - 1] = &inbuf[i];
            }
            break;
        }
    } while (!stat.done && ++i < INBUF_SIZE && argc <= MAX_CMD_ARG);
  
    if (stat.inQuote)
    {
        return 1;
    }
    if (argc < 1)
    {
        return 0;
    }
    
    Serial.printf("\r\n");
    
    for(int i = 0; i < cmd_count; i++)
    {
        if(strcmp(cmds[i].name, argv[0]) == 0)
        {
            cmds[i].function(argc, argv);
            return 0;
        }
    }
    
    Serial.printf("Error:Invalid command: %s\r\n", argv[0]);
    return 0;
}

void cli_main(void)
{
    char inbuf[INBUF_SIZE];
    unsigned int bp = 0;
    
    print_help();
    Serial.print(PROMPT);
    
    while (true) 
    {
        if (!get_input(inbuf, &bp))
        {
            continue;
        }
                
        int ret = handle_input(inbuf);
        if (ret == 1)
        {
            Serial.print("Error:Syntax error\r\n");
        }
        Serial.print(PROMPT);
    }
}
