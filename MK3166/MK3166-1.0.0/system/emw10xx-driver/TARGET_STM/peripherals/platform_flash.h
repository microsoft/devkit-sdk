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


/* Includes ------------------------------------------------------------------*/
#include "mico_board.h"
#include "mico_board_conf.h"

#ifdef USE_MICO_SPI_FLASH
#include "spi_flash.h"
#endif

#ifdef USE_QUAD_SPI_FLASH
#include "spi_flash.h"
#include "spi_flash_internal.h"
#endif

#ifdef USE_QUAD_SPI_FLASH
extern const platform_qspi_t platform_qspi_peripherals[];
#endif


int iflash_init( void );
int iflash_erase( uint32_t device_start_addr, uint32_t device_end_addr );
int iflash_write( volatile uint32_t* device_address, uint32_t* data_addr, uint32_t size );
int iflash_read( unsigned long device_address, void* const data_addr, uint32_t size );

#ifdef MCU_ENABLE_FLASH_PROTECT
int iflash_protect(uint32_t StartAddress, uint32_t EndAddress, bool enable);
#endif



#ifdef USE_MICO_SPI_FLASH
static OSStatus spiFlashErase(uint32_t StartAddress, uint32_t EndAddress);
#endif



#ifdef USE_QUAD_SPI_FLASH

int qsflash_erase( uint32_t StartAddress, uint32_t EndAddress );
int qsflash_init( /*@out@*/ const platform_qspi_t* qspi, const platform_flash_t* flash, /*@out@*/ sflash_handle_t* const handle, sflash_write_allowed_t write_allowed_in );
int qsflash_read_ID( sflash_handle_t* const handle, void* const data_addr );
int qsflash_write( const sflash_handle_t* const handle, unsigned long device_address, const void* const data_addr, unsigned int size );
int qsflash_read( const sflash_handle_t* const handle, unsigned long device_address, void* const data_addr, unsigned int size );
#endif
