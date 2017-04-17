/* mbed Microcontroller Library
 * Copyright (c) 2017 Nuvoton
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

#include "mbed.h"
#include "NAU88C10.h"

/*
NAU88C10::NAU88C10(PinName i2c_sda, PinName i2c_scl, int i2c_addr, PinName i2s_tx, PinName i2s_rx,
         PinName i2s_bclk, PinName i2s_mclk, PinName i2s_lrclk) : m_addr(i2c_addr), m_I2C(i2c_sda, i2c_scl)
{
    m_I2C.frequency(100000);
    //m_I2S(i2s_tx, i2s_rx, i2s_bclk, i2s_mclk, i2s_lrclk)

    m_masterMode = 1;    // codec is master mode and i2s is slave mode
    power();
    //format(8000, 1, 16);
    //rxBuffer = &m_I2S.rxBuffer[0];
}

NAU88C10::NAU88C10(PinName i2c_sda, PinName i2c_scl, int i2c_addr, PinName i2s_tx, PinName i2s_rx, PinName i2s_bclk, PinName i2s_mclk, PinName i2s_lrclk, char i2s_master_enable, char codec_master_enable)
                         : m_addr(i2c_addr), m_I2C(i2c_sda, i2c_scl)
{
    //m_I2S(i2s_tx, i2s_rx, i2s_bclk, i2s_mclk, i2s_lrclk, i2s_master_enable)
    m_I2C.frequency(100000);
    m_masterMode = codec_master_enable;
    
    power();
    //format(8000, 1, 16);
    //rxBuffer = &m_I2S.rxBuffer[0];
}
*/

NAU88C10::NAU88C10() : m_I2C(I2C_SDA, I2C_SCL) {
    m_addr = 0x1A;
}

void NAU88C10::I2C_WriteNAU88C10(uint8_t u8addr, uint16_t u16data)
{
    char g_au8TxData[2];
    
    g_au8TxData[0] = ((u8addr << 1)  | (u16data >> 8));     //addr(7bit) + data(first bit)
    g_au8TxData[1] = (char)(u16data & 0x00FF);              //data(8bit)
    m_I2C.write((m_addr << 1), g_au8TxData, 2, true);
}

/*
* Configure NAU88C10 through I2C interface
*/
void NAU88C10::power(void)
{
    I2C_WriteNAU88C10(0,  0x000);   /* Reset all registers */
    Thread::wait(500);
    
    if (m_masterMode) {
        I2C_WriteNAU88C10(1,  0x03F);
    } else {
        I2C_WriteNAU88C10(1,  0x01F);
    }
    I2C_WriteNAU88C10(2,  0x1BF);   /* Enable L/R Headphone, ADC Mix/Boost, ADC */
    I2C_WriteNAU88C10(3,  0x07F);   /* Enable L/R main mixer, DAC */
    I2C_WriteNAU88C10(4,  0x010);   /* 16-bit word length, I2S format, Stereo */
    I2C_WriteNAU88C10(5,  0x000);   /* Companding control and loop back mode (all disable) */
    I2C_WriteNAU88C10(6,  0x1AD);   /* Divide by 6, 16K */
    I2C_WriteNAU88C10(7,  0x006);   /* 16K for internal filter coefficients */
    I2C_WriteNAU88C10(10, 0x008);   /* DAC soft mute is disabled, DAC oversampling rate is 128x */
    I2C_WriteNAU88C10(14, 0x108);   /* ADC HP filter is disabled, ADC oversampling rate is 128x */
    I2C_WriteNAU88C10(15, 0x1EF);   /* ADC left digital volume control */
    I2C_WriteNAU88C10(16, 0x1EF);   /* ADC right digital volume control */
    
    I2C_WriteNAU88C10(44, 0x033);   /* LMICN/LMICP is connected to PGA */
    I2C_WriteNAU88C10(47, 0x100);   /* Gain value */
    I2C_WriteNAU88C10(48, 0x100);   /* Gain value */
    I2C_WriteNAU88C10(50, 0x001);   /* Left DAC connected to LMIX */
    I2C_WriteNAU88C10(51, 0x001);   /* Right DAC connected to RMIX */
    
    I2C_WriteNAU88C10(0x34, 0x13F);
    I2C_WriteNAU88C10(0x35, 0x13F);
}

void NAU88C10::format(int rate, char count, char length)
{
    int  clockControl;
    char monoOperationEnable;
    char wordLength;
    char mClkDiv;
    char bClkDiv;
    int  audioInterface = 0x010;
    
    if (count >= 2) {
        count = 2;
        monoOperationEnable = 0x0;
    } else {
        /* FIXME */
        count = 2;
        monoOperationEnable = 0x2;
    }

    audioInterface = (audioInterface & 0x1FE) | monoOperationEnable;
    
    if (length == 16) {
        wordLength = 0x0;
    } else if (length == 32) {
        wordLength = 0x3;
    } else {
        /* FIXME */
        wordLength = 0x3;
    }
    audioInterface = (audioInterface & 0x19F) | (wordLength << 5);
    
    if (rate % 11025) {
        I2C_WriteNAU88C10(36, 0x008);    //12.288Mhz
        I2C_WriteNAU88C10(37, 0x00C);
        I2C_WriteNAU88C10(38, 0x093);
        I2C_WriteNAU88C10(39, 0x0E9);
        /* FIXME */
        if (rate > 48000)
            rate = 8000;
        mClkDiv = (48000 * 256 * 2) / (rate * 256);
        bClkDiv = (rate * 256) / (rate * count * length);
    } else {
        I2C_WriteNAU88C10(36, 0x007);    //11.2896Mhz
        I2C_WriteNAU88C10(37, 0x021);
        I2C_WriteNAU88C10(38, 0x161);
        I2C_WriteNAU88C10(39, 0x026);
        /* FIXME */
        if (rate > 44100)
            rate = 11025;
        mClkDiv = (44100 * 256 * 2) / (rate * 256);
        bClkDiv = (rate * 256) / (rate * count * length);
    }
    
    switch (mClkDiv) {
        case 1:
            mClkDiv = 0;
            break;
        case 2:
            mClkDiv = 2;
            break;
        case 3:
            mClkDiv = 3;
            break;
        case 4:
            mClkDiv = 4;
            break;
        case 6:
            mClkDiv = 5;
            break;
        case 8:
            mClkDiv = 6;
            break;
        case 12:
            mClkDiv = 7;
            break;
    }
    
    switch (bClkDiv) {
        case 1:
            bClkDiv = 0;
            break;
        case 2:
            bClkDiv = 1;
            break;
        case 4:
            bClkDiv = 2;
            break;
        case 8:
            bClkDiv = 3;
            break;
        case 16:
            bClkDiv = 4;
            break;
        case 32:
            bClkDiv = 5;
            break;
    }
    
    clockControl = 0;
    clockControl = (clockControl & 0x0FF);
    clockControl = (clockControl & 0x1FE);
    if (m_masterMode) {
        clockControl = clockControl | (1 << 8);
        clockControl = clockControl | (1 << 0);
    }
    
    clockControl = (clockControl & 0x11F) | (mClkDiv << 5);
    clockControl = (clockControl & 0x1E3) | (bClkDiv << 2);
    
    I2C_WriteNAU88C10(4,  audioInterface);  
    I2C_WriteNAU88C10(6,  clockControl);
    
    /* TODO
    if (monoOperationEnable == 0) {
        m_I2S.format(rate, 2, length);
    } else {
        m_I2S.format(rate, 1, length);;
    }
    */
}

void NAU88C10::startRecord(void)
{
    //TODO
}

void NAU88C10::read(void)
{
    //TODO
}


void NAU88C10::stop(void)
{
    //TODO
}

void NAU88C10::write(int *buffer, int from, int length)
{
    //TODO
}

void NAU88C10::attach(void(*fptr)(void))
{
    //TODO
}

void NAU88C10::attach(Callback<void()> func)
{
    //TODO
}

void NAU88C10::startPlaying(void)
{
    //TODO
}