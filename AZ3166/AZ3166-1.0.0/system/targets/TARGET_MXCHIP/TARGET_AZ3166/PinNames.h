/* mbed Microcontroller Library
 * Copyright (c) 2016 ARM Limited
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
#ifndef MBED_PINNAMES_H
#define MBED_PINNAMES_H

#include "cmsis.h"
#include "PinNamesTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PA_0  = 0x00,
    PA_1  = 0x01,
    PA_2  = 0x02,
    PA_3  = 0x03,
    PA_4  = 0x04,
    PA_5  = 0x05,
    PA_6  = 0x06,
    PA_7  = 0x07,
    PA_8  = 0x08,
    PA_9  = 0x09,
    PA_10 = 0x0A,
    PA_11 = 0x0B,
    PA_12 = 0x0C,
    PA_13 = 0x0D,
    PA_14 = 0x0E,
    PA_15 = 0x0F,

    PB_0  = 0x10,
    PB_1  = 0x11,
    PB_2  = 0x12,
    PB_3  = 0x13,
    PB_4  = 0x14,
    PB_5  = 0x15,
    PB_6  = 0x16,
    PB_7  = 0x17,
    PB_8  = 0x18,
    PB_9  = 0x19,
    PB_10 = 0x1A,
    PB_11 = 0x1B,
    PB_12 = 0x1C,
    PB_13 = 0x1D,
    PB_14 = 0x1E,
    PB_15 = 0x1F,

    PC_0  = 0x20,
    PC_1  = 0x21,
    PC_2  = 0x22,
    PC_3  = 0x23,
    PC_4  = 0x24,
    PC_5  = 0x25,
    PC_6  = 0x26,
    PC_7  = 0x27,
    PC_8  = 0x28,
    PC_9  = 0x29,
    PC_10 = 0x2A,
    PC_11 = 0x2B,
    PC_12 = 0x2C,
    PC_13 = 0x2D,
    PC_14 = 0x2E,
    PC_15 = 0x2F,

    PD_0  = 0x30,
    PD_1  = 0x31,
    PD_2  = 0x32,
    PD_3  = 0x33,
    PD_4  = 0x34,
    PD_5  = 0x35,
    PD_6  = 0x36,
    PD_7  = 0x37,
    PD_8  = 0x38,
    PD_9  = 0x39,
    PD_10 = 0x3A,
    PD_11 = 0x3B,
    PD_12 = 0x3C,
    PD_13 = 0x3D,
    PD_14 = 0x3E,
    PD_15 = 0x3F,

    PE_0  = 0x40,
    PE_1  = 0x41,
    PE_2  = 0x42,
    PE_3  = 0x43,
    PE_4  = 0x44,
    PE_5  = 0x45,
    PE_6  = 0x46,
    PE_7  = 0x47,
    PE_8  = 0x48,
    PE_9  = 0x49,
    PE_10 = 0x4A,
    PE_11 = 0x4B,
    PE_12 = 0x4C,
    PE_13 = 0x4D,
    PE_14 = 0x4E,
    PE_15 = 0x4F,

    PF_0  = 0x50,
    PF_1  = 0x51,
    PF_2  = 0x52,
    PF_3  = 0x53,
    PF_4  = 0x54,
    PF_5  = 0x55,
    PF_6  = 0x56,
    PF_7  = 0x57,
    PF_8  = 0x58,
    PF_9  = 0x59,
    PF_10 = 0x5A,
    PF_11 = 0x5B,
    PF_12 = 0x5C,
    PF_13 = 0x5D,
    PF_14 = 0x5E,
    PF_15 = 0x5F,

    PG_0  = 0x60,
    PG_1  = 0x61,
    PG_2  = 0x62,
    PG_3  = 0x63,
    PG_4  = 0x64,
    PG_5  = 0x65,
    PG_6  = 0x66,
    PG_7  = 0x67,
    PG_8  = 0x68,
    PG_9  = 0x69,
    PG_10 = 0x6A,
    PG_11 = 0x6B,
    PG_12 = 0x6C,
    PG_13 = 0x6D,
    PG_14 = 0x6E,
    PG_15 = 0x6F,

    PH_0  = 0x70,
    PH_1  = 0x71,

    // ADC internal channels
    ADC_TEMP = 0xF0,
    ADC_VREF = 0xF1,
    ADC_VBAT = 0xF2,

    // EMW3166 qspi flasgh pins
    QSPI_CS  = PC_11,
    QSPI_CLK = PB_1,
    QSPI_D0  = PA_6,
    QSPI_D1  = PA_7,
    QSPI_D2  = PC_4,
    QSPI_D3  = PC_5,

    // EMW3166 sdio wlan pins
    SDIO_CLK  = PC_12,
    SDIO_CMD  = PD_2,
    SDIO_D0   = PC_8,
    SDIO_D1   = PC_9,
    SDIO_D2   = PC_10,
    SDIO_D3   = PB_5,
    SDIO_OOB_IRQ   = PC_0,

    // EMW3166 pins
    MBED_GPIO_2  = PB_2,
    MBED_GPIO_4  = PB_15,
    MBED_GPIO_5  = PB_12,
    MBED_GPIO_6  = PB_13,
    MBED_GPIO_7  = PB_14,
    MBED_GPIO_8  = PC_6,
    MBED_GPIO_9  = PA_15,
    MBED_GPIO_12 = PC_7,
    MBED_GPIO_14 = PC_0,
    MBED_GPIO_16 = PC_13,
    MBED_GPIO_17 = PB_8,
    MBED_GPIO_18 = PB_9,
    MBED_GPIO_19 = PB_10,
    MBED_GPIO_27 = PB_3,  
    MBED_GPIO_29 = PB_7,
    MBED_GPIO_30 = PB_6,
    MBED_GPIO_31 = PB_4,
    MBED_GPIO_33 = PA_10,
    MBED_GPIO_34 = PA_5,
    MBED_GPIO_35 = PA_11,
    MBED_GPIO_36 = PA_12,
    MBED_GPIO_37 = PB_0,
    MBED_GPIO_38 = PA_4,

    // Arduino connector namings
    A0          = (int)0xFFFFFFFF,
    A1          = (int)0xFFFFFFFF,
    A2          = MBED_GPIO_38,
    A3          = MBED_GPIO_34,
    A4          = (int)0xFFFFFFFF,
    A5          = (int)0xFFFFFFFF,
    D0          = MBED_GPIO_29,
    D1          = MBED_GPIO_30,
    D2          = (int)0xFFFFFFFF,
    D3          = (int)0xFFFFFFFF,
    D4          = MBED_GPIO_19,
    D5          = MBED_GPIO_16,
    D6          = MBED_GPIO_14,
    D7          = (int)0xFFFFFFFF,
    D8          = MBED_GPIO_2,
    D9          = MBED_GPIO_27,
    D10         = MBED_GPIO_5,
    D11         = MBED_GPIO_4,
    D12         = MBED_GPIO_7,
    D13         = MBED_GPIO_6,
    D14         = MBED_GPIO_18,
    D15         = MBED_GPIO_17,

    // Generic signals namings
    LED1            = MBED_GPIO_2,
    LED2            = MBED_GPIO_16,
    LED3            = MBED_GPIO_14,
    LED_RED         = LED3,
    LED_WIFI        = LED1,
    LED_AZURE       = LED2,
    LED_USER        = LED3,
    RGB_R           = MBED_GPIO_31,
    RGB_G           = MBED_GPIO_27,
    RGB_B           = MBED_GPIO_12,
    USER_BUTTON_A   = MBED_GPIO_37,
    USER_BUTTON_B   = MBED_GPIO_33,
    USER_BUTTON     = USER_BUTTON_A,
    SERIAL_TX       = MBED_GPIO_30,
    SERIAL_RX       = MBED_GPIO_29,
    USBTX           = SERIAL_TX,
    USBRX           = SERIAL_RX,
    I2C_SCL         = MBED_GPIO_17,
    I2C_SDA         = MBED_GPIO_18,
    SPI_MOSI        = MBED_GPIO_4,
    SPI_MISO        = MBED_GPIO_7,
    SPI_SCK         = MBED_GPIO_6,
    SPI_CS          = MBED_GPIO_5,
    PWM_OUT         = D9,

    // Not connected
    NC = (int)0xFFFFFFFF
} PinName;

#ifdef __cplusplus
}
#endif

#endif
