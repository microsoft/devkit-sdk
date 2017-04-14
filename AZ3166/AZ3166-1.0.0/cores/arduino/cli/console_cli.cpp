/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "mbed.h"
#include <stdio.h>
#include "EEPROMInterface.h"
#include "EMW10xxInterface.h"
#include "UARTClass.h"
#include "console_cli.h"
#include "mico.h"

struct console_command 
{
    const char *name;
    const char *help;
    void (*function) (int argc, char **argv);
};

#define MAX_CMD_ARG         4

#define WIFI_SSID_MAX_LEN   32
#define WIFI_PWD_MAX_LEN    64
#define AZ_IOT_HUB_MAX_LEN  256

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
static void wifi_ssid_command(int argc, char **argv);
static void wifi_pwd_Command(int argc, char **argv);
static void az_iothub_command(int argc, char **argv);

static const struct console_command cmds[] = {
  {"help",          "Help document",                                            help_command},
  {"version",       "System version",                                           get_version_command},
  {"exit",          "Exit and reboot",                                          reboot_and_exit_command},
  {"set_wifissid",  "Set Wi-Fi SSID",                                           wifi_ssid_command},
  {"set_wifipwd",   "Set Wi-Fi password",                                       wifi_pwd_Command},
  {"set_az_iothub", "Set the connection string of Microsoft Azure IOT hub",     az_iothub_command},
};

static const int cmd_count = sizeof(cmds) / sizeof(struct console_command);

////////////////////////////////////////////////////////////////////////////////////
// Command handlers
static void print_help()
{
    Serial.print("\r\n=========================================================");
    Serial.print("\r\n= Configuration console                                 =");
    Serial.print("\r\n=========================================================\r\n");
    
    for (int i = 0; i < cmd_count; i++)
    {
        Serial.printf(" %s: %s.\r\n", cmds[i].name, cmds[i].help);
    }
}

static void help_command(int argc, char **argv)
{
    print_help();
}

static void get_version_command(int argc, char **argv)
{
    char ver[128];
    uint8_t major, minor, revision;
    int ret;
    
    mico_sdk_version( &major, &minor, &revision );
    
    Serial.printf( "Kernel version: %s\r\n", MicoGetVer() );
    Serial.printf( "SDK version: %d.%d.%d\r\n", major, minor, revision );
    
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

static void reboot_and_exit_command(int argc, char **argv)
{
    Serial.printf("Reboot\r\n");
    mico_system_reboot();
}

static void write_eeprom(char* string, int idxZone)
{    
    EEPROMInterface eeprom;
    int len = strlen(string) + 1;
    
    // Write data to EEPROM
    ResponseCode responseCode = eeprom.write((uint8_t*)string, len, idxZone);
    if (responseCode != OK)
    {
        Serial.printf("ERROR: Failed to write EEPROM: 0x%02x.\r\n", responseCode);
        return;
    }
    
    // Verify
    uint8_t *pBuff = (uint8_t*)malloc(len);
    int result = eeprom.read(pBuff, len, idxZone);
    if (result != len || strncmp(string, (char*)pBuff, len) != 0)
    {
        Serial.printf("ERROR: Verify failed.\r\n");
    }
    free(pBuff);
}

static void wifi_ssid_command(int argc, char **argv)
{
    if (argc == 1 || argv[1] == NULL) 
    {
        Serial.printf("Usage: set_wifissid <SSID>. Please provide the SSID of the Wi-Fi.\r\n");
        return;
    }
    int len = strlen(argv[1]);
    if (len == 0 || len > WIFI_SSID_MAX_LEN)
    {
        Serial.printf("Invalid Wi-Fi SSID.\r\n");
    }
    
    write_eeprom(argv[1], 0x03);
    Serial.printf("INFO: Set Wi-Fi SSID successfully.\r\n");
}

static void wifi_pwd_Command(int argc, char **argv)
{
    if (argc == 1 || argv[1] == NULL) 
    {
        Serial.printf("Usage: set_wifipwd <password>. Please provide the password of the Wi-Fi.\r\n");
        return;
    }
    int len = strlen(argv[1]);
    if (len == 0 || len > WIFI_PWD_MAX_LEN)
    {
        Serial.printf("Invalid Wi-Fi password.\r\n");
    }
    
    write_eeprom(argv[1], 0x0A);
    Serial.printf("INFO: Set Wi-Fi password successfully.\r\n");
}

static void az_iothub_command(int argc, char **argv)
{
    if (argc == 1 || argv[1] == NULL) 
    {
        Serial.printf("Usage: set_az_iothub <connection string>. Please provide the connection string of the Azure IoT hub.\r\n");
        return;
    }
    int len = strlen(argv[1]);
    if (len == 0 || len > AZ_IOT_HUB_MAX_LEN)
    {
        Serial.printf("Invalid Azure IoT hub connection string.\r\n");
    }
    
    write_eeprom(argv[1], 0x05);
    Serial.printf("INFO: Set Azure Iot hub connection string successfully.\r\n");
}

////////////////////////////////////////////////////////////////////////////////////
// Console app
static bool get_input(char *inbuf, unsigned int *bp)
{
    if (inbuf == NULL) 
    {
        return false;
    }
    
    while (true) 
    {
        while (!Serial.available())
        {
            wait_ms(50);
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
        Serial.write(inbuf[*bp]);
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
    
    EEPROMInterface eeprom;
    
    uint8_t *pSSID = (uint8_t*)malloc(WIFI_SSID_MAX_LEN);
    int responseCode = eeprom.read(pSSID, WIFI_SSID_MAX_LEN, 0x03);

    if(responseCode)
    {
        EMW10xxInterface wlan;
        int ret;

        Serial.printf("Trying to connect to Wifi. Current Wifi SSID is %s\n",pSSID);
        uint8_t *pPassword = (uint8_t*)malloc(WIFI_PWD_MAX_LEN);
        responseCode = eeprom.read(pPassword, WIFI_PWD_MAX_LEN, 0x0A);
        if(responseCode)
        {
            ret = wlan.connect( (char*)pSSID, (char*)pPassword, NSAPI_SECURITY_WPA_WPA2, 0 );
        }
        else
        {
            //empty password
            ret = wlan.connect( (char*)pSSID, "" , NSAPI_SECURITY_WPA_WPA2, 0 );          
        }
        
        if ( ret != NSAPI_ERROR_OK ) 
        {
            Serial.printf("Wifi connection failed, please reset SSID and password\r\n");
        }
        else
        {
            Serial.printf("Wifi connected successfully.\r\n");
        }        
    }

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