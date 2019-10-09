/* MiCO Team
 * Copyright (c) 2017 MXCHIP Information Tech. Co.,Ltd
 *
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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mico_common.h"
#include "mico_rtos.h"
#include "mico_wlan.h"

#define CONFIG_MODE_EASYLINK                    (1)
#define CONFIG_MODE_SOFT_AP                     (2)
#define CONFIG_MODE_EASYLINK_WITH_SOFTAP        (3)
#define CONFIG_MODE_WAC                         (4)
#define CONFIG_MODE_USER                        (5)
#define CONFIG_MODE_AWS                         (6)

#ifndef MICO_PREBUILT_LIBS
#include "mico_config.h"
#endif

#include "mico_board_conf.h"

#define MICO_COMPLETE_SYSTEM          (0)

#ifndef MiCO_SDK_VERSION_MAJOR
#define MiCO_SDK_VERSION_MAJOR      (3)
#endif

#ifndef MiCO_SDK_VERSION_MINOR
#define MiCO_SDK_VERSION_MINOR      (0)
#endif

#ifndef MiCO_SDK_VERSION_REVISION
#define MiCO_SDK_VERSION_REVISION   (0)
#endif

#if MICO_WLAN_CONFIG_MODE != CONFIG_MODE_EASYLINK
#define EasyLink_Needs_Reboot
#endif

#define maxSsidLen          32
#define maxKeyLen           64
#define maxNameLen          32
#define maxIpLen            16

/* For legacy definition */
#ifndef MICO_WLAN_CONFIG_MODE
#define MICO_WLAN_CONFIG_MODE     MICO_CONFIG_MODE
#endif

typedef enum
{
  eState_Normal,
  eState_Software_Reset,
  eState_Wlan_Powerdown,
  eState_Restore_default,
  eState_Standby,
} system_state_t;

enum  config_state_type_e{
  /*All settings are in default state, module will enter easylink mode if CONFIG_MODE_TRIGGER_AUTO is selected.
  Press down Easyink button for 3 seconds (defined by RestoreDefault_TimeOut) to enter this mode */
  unConfigured,
#ifdef EasyLink_Needs_Reboot
  /*Module will enter easylink mode temporally when powered on, and go back to allConfigured
    mode after time out (Defined by EasyLink_TimeOut), This mode is used for changing wlan
    settings if module is moved to a new wlan environment. Press down Easyink button to
    enter this mode */                
  wLanUnConfigured,
#endif
  /*Normal working mode, module use the configured settings to connect to wlan, and run
    user's threads*/
  allConfigured,
  /*If MFG_MODE_AUTO is enabled and MICO settings are erased (maybe a fresh device just has 
    been programmed or MICO settings is damaged), module will enter MFG mode when powered on. */
  mfgConfigured,
#ifdef EasyLink_Needs_Reboot
  /*All settings are in default state and triggered by external, module will enter easylink
    mode. Trigger source is selected by MICO_WLAN_CONFIG_MODE_TRIGGER */
  unConfigured2
#endif
};
typedef uint8_t config_state_type_t;

/* OTA should save this table to flash */
typedef struct  _boot_table_t {
  uint32_t start_address; // the address of the bin saved on flash.
  uint32_t length; // file real length
  uint8_t version[8];
  uint8_t type; // B:bootloader, P:boot_table, A:application, D: 8782 driver
  uint8_t upgrade_type; //u:upgrade, 
  uint16_t crc;
  uint8_t reserved[4];
}boot_table_t;

typedef struct _mico_sys_config_t
{
  /*Device identification*/
  char            name[maxNameLen];

  /*Wi-Fi configuration*/
  char            ssid[maxSsidLen];
  char            user_key[maxKeyLen]; 
  int             user_keyLength;
  char            key[maxKeyLen]; 
  int             keyLength;
  char            bssid[6];
  int             channel;
  wlan_sec_type_t security;

  /*Power save configuration*/
  bool            rfPowerSaveEnable;
  bool            mcuPowerSaveEnable;

  /*Local IP configuration*/
  bool            dhcpEnable;
  char            localIp[maxIpLen];
  char            netMask[maxIpLen];
  char            gateWay[maxIpLen];
  char            dnsServer[maxIpLen];

  /*EasyLink configuration*/
  config_state_type_t   configured;
  uint8_t               easyLinkByPass;
  uint32_t              reserved;

  /*Services in MICO system*/
  uint32_t        magic_number;

  /*Update seed number when configuration is changed*/
  int32_t         seed;
} mico_sys_config_t;


#ifdef MICO_BLUETOOTH_ENABLE
#define MICO_BT_PARA_LOCAL_KEY_DATA  65  /* BTM_SECURITY_LOCAL_KEY_DATA_LEN */

#define MICO_BT_DCT_NAME            249
#define MICO_BT_DCT_MAX_KEYBLOBS    146   /* Maximum size of key blobs to be stored :=  size of BR-EDR link keys +  size of BLE keys*/
#define MICO_BT_DCT_ADDR_FIELD      6
#define MICO_BT_DCT_LENGTH_FIELD    2
#ifndef MICO_BT_DCT_MAX_DEVICES
#define MICO_BT_DCT_MAX_DEVICES     10    /* Maximum number of device records stored in nvram */
#endif
#define MICO_BT_DCT_ADDR_TYPE       1
#define MICO_BT_DCT_DEVICE_TYPE     1

/* Length of BD_ADDR + 2bytes length field */
#define MICO_BT_DCT_ENTRY_HDR_LENGTH  (MICO_BT_DCT_ADDR_FIELD + MICO_BT_DCT_LENGTH_FIELD + MICO_BT_DCT_ADDR_TYPE + MICO_BT_DCT_DEVICE_TYPE)

#define MICO_BT_DCT_LOCAL_KEY_OFFSET  OFFSETOF( mico_bt_config_t, bluetooth_local_key )
#define MICO_BT_DCT_REMOTE_KEY_OFFSET OFFSETOF( mico_bt_config_t, bluetooth_remote_key )

#pragma pack(1)
typedef struct
{
//    uint8_t bluetooth_local_addeess[6];
//    uint8_t bluetooth_local_name[249]; /* including null termination */
    uint8_t bluetooth_local_key[MICO_BT_PARA_LOCAL_KEY_DATA];
    uint8_t bluetooth_remote_key[MICO_BT_DCT_ENTRY_HDR_LENGTH + MICO_BT_DCT_MAX_KEYBLOBS][MICO_BT_DCT_MAX_DEVICES];
    uint8_t padding[1];   /* to ensure 32-bit aligned size */
} mico_bt_config_t;
#pragma pack()
#endif


typedef struct {

  /*OTA options*/
  boot_table_t             bootTable;

  /*MICO system core configuration*/
  mico_sys_config_t        micoSystemConfig;

#ifdef MICO_BLUETOOTH_ENABLE
  mico_bt_config_t         bt_config; 
#endif
} system_config_t;

typedef struct
{
  system_state_t        current_sys_state;
  mico_semaphore_t      sys_state_change_sem;
  /*MICO system Running status*/
  char                  localIp[maxIpLen];
  char                  netMask[maxIpLen];
  char                  gateWay[maxIpLen];
  char                  dnsServer[maxIpLen];
  char                  mac[18];
  char                  rf_version[50];
} system_status_wlan_t;

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif



