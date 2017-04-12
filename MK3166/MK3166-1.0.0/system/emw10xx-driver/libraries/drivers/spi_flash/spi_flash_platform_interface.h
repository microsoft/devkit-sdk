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

#ifndef INCLUDED_SPI_FLASH_PLATFORM_INTERFACE_H
#define INCLUDED_SPI_FLASH_PLATFORM_INTERFACE_H

#include "mico_common.h"

#ifdef __cplusplus
 extern "C" {
#endif


 typedef struct
 {
     /*@null@*/ /*@observer@*/  const void*   tx_buffer;
     /*@null@*/ /*@dependent@*/ void*         rx_buffer;
                                unsigned long length;
 } sflash_platform_message_segment_t;

extern int sflash_platform_init      ( /*@shared@*/ void* peripheral_id, /*@out@*/ void** platform_peripheral_out );
extern int sflash_platform_send_recv ( const void* platform_peripheral, /*@in@*/ /*@out@*/ sflash_platform_message_segment_t* segments, unsigned int num_segments  );
extern int sflash_platform_deinit    ( void );

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SPI_FLASH_PLATFORM_INTERFACE_H */
