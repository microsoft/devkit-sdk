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

#ifndef __SYSTEM_INTERNAL_H__
#define __SYSTEM_INTERNAL_H__

#include "system.h"

#ifndef MICO_SYSTEM_LOG_INFO_DISABLE
  #define system_log(M, ...) custom_log("SYSTEM", M, ##__VA_ARGS__)
  #define system_log_trace() custom_log_trace("SYSTEM")
#else
  #define system_log(M, ...)
  #define system_log_trace()
#endif

/* Define MICO service thread stack size */
#define STACK_SIZE_LOCAL_CONFIG_SERVER_THREAD   0x400
#define STACK_SIZE_LOCAL_CONFIG_CLIENT_THREAD   0x1650
#define STACK_SIZE_NTP_CLIENT_THREAD            0x450
#define STACK_SIZE_mico_system_MONITOR_THREAD   0x300

#define EASYLINK_BYPASS_NO                      (0)
#define EASYLINK_BYPASS                         (1)
#define EASYLINK_SOFT_AP_BYPASS                 (2)



#define SYS_MAGIC_NUMBR     (0xA43E2165)

typedef struct _mico_Context_t
{
  /*Flash content*/
  system_config_t           flashContentInRam;
  mico_mutex_t              flashContentInRam_mutex;

  void *                    user_config_data;
  uint32_t                  user_config_data_size;

  /*Running status*/
  system_status_wlan_t      micoStatus;
} system_context_t;

OSStatus system_notification_init( system_context_t * const inContext);

OSStatus system_network_daemen_start( system_context_t * const inContext );

OSStatus system_discovery_init( system_context_t * const inContext );

void system_connect_wifi_normal( system_context_t * const inContext );

void system_connect_wifi_fast( system_context_t * const inContext);

OSStatus system_easylink_wac_start( system_context_t * const inContext );

OSStatus system_easylink_wac_stop( void );

OSStatus system_easylink_start( system_context_t * const inContext );

OSStatus MICORestoreMFG                 ( void );

OSStatus MICOReadConfiguration          ( system_context_t * const inContext );

system_context_t *system_context( void );

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif

