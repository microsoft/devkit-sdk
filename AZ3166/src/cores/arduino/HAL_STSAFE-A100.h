/**
 *******************************************************************************************************************************************************
 * @file    HAL_STSAFE-A100.h
 * @author  STMicroelectronics
 * @version HAL_STSAFE-A100 1.0.0
 * @date    19th October 2017
 * @brief   Definitions for the library
 *******************************************************************************************************************************************************
 * @attention
 * Ultimate Liberty License Agreement v.1 for use of STSAFE-A software in Microsoft Azure IoT Developer Kit
 *
 * Copyright © 2017 STMicroelectronics International N.V.. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted, provided that the following conditions are met:
 *
 * 1.     Redistribution of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * 2.     Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
 * 		  documentation and/or other materials provided with the distribution.
 * 3.     Neither the name of STMicroelectronics nor the names of other contributors to this software may be used to endorse or promote products
 * 		  derived from this software without specific written permission.
 * 4.     This software, including modifications and/or derivative works of this software, must execute solely and exclusively on microcontroller
 * 		  devices manufactured by or for STMicroelectronics or on a microcontroller manufactured by a third party that is used solely and exclusively
 * 		  in combination with an integrated circuit manufactured by or for STMicroelectronics.
 * 5.     Redistribution and use of this software other than as permitted under this license is void and will automatically terminate your rights
 * 		  under this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * OF THIRD PARTY INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT SHALL STMICROELECTRONICS
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************************
 */


#ifndef HAL_STSAFEA100_H
#define HAL_STSAFEA100_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported functions ------------------------------------------------------- */

/*!
 * \brief Initialize the library
 *
 * This command must be executed one time before using other HAL functions
 * It set communication with STSAFE-A100, verify if STSAFE-A100 is personalized or not
 *
 * \param in  : i2c_address : i2c address of STSAFE-A100
 * \param out : handle_se : Handle for STSAFE-A100
 * \return 0 if no error else 1
 *
 */

uint8_t Init_HAL(uint8_t i2c_address,void **handle_se);

/*!
 * \brief Run personalization of chip
 *
 * This command must be executed one time in the life of the product
 * and does personalization of STSAFE-A100
 *
 * \param in : handle_se : Handle for STSAFE-A100
 * \param in : perso_type define which personalization to be ran :
 *
 *				perso_type=0: use default keys and meanwhile set buf = NULL
 *				perso_type=1: use random generated keys and meanwhile set buf = NULL
 *				perso_type=2: use the keys customers prefer and meanwhile set buf = preferred key.
 *
 * \param in : pcrop_enable define if pcrop must be activated or not :
 * 			   pcrop_enable=0 => pcrop is not enabled
 * 			   pcrop_enable=1 => pcrop is enabled on dedicated sector
 *
 *
 * \param in : buff : Buffer which contains host keys Only used if perso_type=2
 *
 * \return 0 if no error else 1
 *
 */

uint8_t Init_Perso(void *handle_se, int perso_type, int pcrop_enable, uint8_t* buf);

/*!
 * \brief Free memory allocated by handle
 *
 * This command free memory allocated by Handle
 *
 * \param in : handle_se : Handle for STSAFE-A100
 * \return 0 if no error else 1
 *
 */

uint8_t Free_HAL(void *handle_se);

/*!
 * \brief Store data in STSAFE-A100
 *
 * This command allow to store unencrypted data inside zone

 * \param in : handle_se : Handle for STSAFE-A100
 * \param in : zone : zone number
 * \param in : size : Size of data to store
 * \param in : in_Data : data to store
 * \param in : in_Data : offset
 *
 * \return 0 if no error else 1
 *
 */

uint8_t HAL_Store_Data_Zone (void *handle_se,uint8_t zone,uint16_t size,uint8_t* in_Data, uint16_t offset);

/*!
 * \brief Store data encrypted in STSAFE-A100
 *
 * This command allow to store data encrypted inside zone
 * Max data that could be stored is 480 bytes
 *
 * \param in : handle_se : Handle for STSAFE-A100
 * \param in : zone : zone number
 * \param in : size : Size of data to store
 * \param in : in_Data : data to store
 * \param in : in_Data : offset
 *
 * Warning : Writing X bytes to Y offset inside an envelope result of writing 8+X bytes at offset Y
 *
 * Example of write and read data's within envelope at contiguous offset
 * StatusCode=HAL_Store_Data_WithinEnvelop(handle_se,8,480,Array_1,0);
 * StatusCode=HAL_Store_Data_WithinEnvelop(handle_se,ZONE_DATA,100,Array_2,480+8);
 * StatusCode=HAL_Get_Data_WithinEnvelop(handle_se,ZONE_DATA,480, Array_Out_1,0);
 * StatusCode=HAL_Get_Data_WithinEnvelop(handle_se,ZONE_DATA,100, Array_Out_2,480+8);
 *
 *
 *
 * \return 0 if no error else 1
 */

uint8_t HAL_Store_Data_WithinEnvelop (void *handle_se,uint8_t zone,uint16_t size,uint8_t* in_Data, uint16_t offset);

/*!
 * \brief Get data inside zone
 *
 * This command allow to get data encrypted inside STSAFE-A100
 * Max data that could be retrieved  is 800 bytes
 *
 * \param in : handle_se : Handle for STSAFE-A100
 * \param in : zone      : zone number
 * \param in : size      : Size of data read from zone
 * \param out: buf       : Data read from zone
 * \param in : in_Data	 : offset
 * \return 0 if no error else 1
 */

uint8_t HAL_Get_Data_Zone (void *handle_se,uint8_t zone,uint16_t size, uint8_t* buf, uint16_t offset);

/*!
 * \brief Get data encrypted inside zone
 *
 * This command allow to store data encrypted inside STSAFE-A100
 * Max data that could be retrieved  is 480 bytes
 *
 * \param in : handle_se : Handle for STSAFE-A100
 * \param in : zone      : zone number
 * \param in : size      : Size of datas read from zone
 * \param out: buf       : Data read from zone
 * \param in : in_Data 	 : offset
 *
 * Warning : if another contiguous envelope was written in zone before, pay attention to read data adding 8 bytes to offset
 *
 * Example of write and read data's within envelope at contiguous offset
 * StatusCode=HAL_Store_Data_WithinEnvelop(handle_se,8,480,Array_1,0);
 * StatusCode=HAL_Store_Data_WithinEnvelop(handle_se,ZONE_DATA,100,Array_2,480+8);
 * StatusCode=HAL_Get_Data_WithinEnvelop(handle_se,ZONE_DATA,480, Array_Out_1,0);
 * StatusCode=HAL_Get_Data_WithinEnvelop(handle_se,ZONE_DATA,100, Array_Out_2,480+8);
 *
 *
 * \return 0 if no error else 1
 *
 */
uint8_t HAL_Get_Data_WithinEnvelop (void *handle_se,uint8_t zone,uint16_t size, uint8_t* buf, uint16_t offset);

/*!
 * \brief Erase datas in STSAFE-A100 zone
 *
 * This command set to 0x0 STSAFE-A100 zone
 *
 * Example : for size=5, it set first 5 bytes of zone to 0x0

 * \param in : handle_se : Handle for STSAFE-A100
 * \param in : zone      : zone number
 * \param in : size      : size of data to be erased
 * \param in : in_Data 	 : offset
*/

uint8_t HAL_Erase_Data_Zone (void *handle_se,uint8_t zone,uint16_t size, uint16_t offset);

/*!
 * \brief Get Library version
 *
 * This command must be executed one time in the life of the product

 * \param out :  Library version
 *
 * \return 0 if no error else 1
 *
 */

void HAL_Version(char *string);

#ifdef __cplusplus
}

#endif

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

