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


/** @mainpage MICO 

    This documentation describes the MICO APIs.
    It consists of:
     - MICO Core APIs   
     - MICO Hardware Abstract Layer APIs    
     - MICO Algorithm APIs        
     - MICO System APIs        
     - MICO Middleware APIs
     - MICO Drivers interface
 */

#ifndef __MICO_H_
#define __MICO_H_

/* MiCO SDK APIs */
#include "mico_debug.h"
#include "mico_common.h"
#include "mico_rtos.h"
#include "mico_wlan.h"
#include "mico_socket.h"
#include "mico_system.h"
#include "mico_platform.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MicoGetRfVer                wlan_driver_version
#define MicoGetVer                  system_lib_version
#define MicoInit                    mxchipInit

/** @defgroup MICO_Core_APIs MICO Core APIs
  * @brief MiCO Initialization, RTOS, TCP/IP stack, and Network Management
  */

/** @addtogroup MICO_Core_APIs
  * @{
  */

/** \defgroup MICO_Init_Info Initialization and Tools
  * @brief Get MiCO version or RF version, flash usage information or init MiCO TCPIP stack
  * @{
 */

 /******************************************************
 *                    Structures
 ******************************************************/



/******************************************************
 *              Function Declarations
 ******************************************************/

/**
  * @brief  Get RF driver's version.
  *
  * @note   Create a memery buffer to store the version characters.
  *         THe input buffer length should be 40 bytes at least.
  * @note   This must be executed after micoInit().
  * @param  inVersion: Buffer address to store the RF driver. 
  * @param  inLength: Buffer size. 
  *
  * @return int
  */
int MicoGetRfVer( char* outVersion, uint8_t inLength );

/**
  * @brief  Get MICO's version.
  *
  * @param  None 
  *
  * @return Point to the MICO's version string.
  */
char* MicoGetVer( void );

/**
  * @brief  Initialize the TCPIP stack thread, RF driver thread, and other
            supporting threads needed for wlan connection. Do some necessary
            initialization
  *
  * @param  None
  *
  * @return kNoErr: success, kGeneralErr: fail
  */
OSStatus MicoInit( void );


/**
  * @brief  Get an identifier id from device, every id is unique and will not change in life-time
  *
  * @param  identifier length 
  *
  * @return Point to the identifier 
  */
const uint8_t* mico_generate_cid( uint8_t *length );


#ifdef __cplusplus
	}
#endif

#endif /* __MICO_H_ */

/**
  * @}
  */

/**
  * @}
  */

