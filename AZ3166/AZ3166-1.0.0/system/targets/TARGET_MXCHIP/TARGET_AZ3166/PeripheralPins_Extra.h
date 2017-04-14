/* mbed Microcontroller Library
 *******************************************************************************
 * Copyright (c) 2014, STMicroelectronics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 */

#ifndef MBED_PERIPHERALPINS_EXTRA_H
#define MBED_PERIPHERALPINS_EXTRA_H

#include "pinmap.h"
#include "PeripheralNames.h"

//*** QSPI ***
#ifdef DEVICE_QSPI
extern const PinMap PinMap_QSPI_D0[];
extern const PinMap PinMap_QSPI_D1[];
extern const PinMap PinMap_QSPI_D2[];
extern const PinMap PinMap_QSPI_D3[];
extern const PinMap PinMap_QSPI_CLK[];
extern const PinMap PinMap_QSPI_CS[];
#endif

//*** SDIO ***
#ifdef DEVICE_SDIO
extern const PinMap PinMap_SDIO_CLK[];
extern const PinMap PinMap_SDIO_CMD[];
extern const PinMap PinMap_SDIO_D0[];
extern const PinMap PinMap_SDIO_D1[];
extern const PinMap PinMap_SDIO_D2[];
extern const PinMap PinMap_SDIO_D3[];
#endif

#endif
