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

#ifndef MBED_NAU88C10_H
#define MBED_NAU88C10_H

#include "mbed.h"
//#include "I2S.h"

class NAU88C10
{        
    public:
        NAU88C10(PinName i2c_sda, PinName i2c_scl, int i2c_addr, PinName i2s_tx, PinName i2s_rx, PinName i2s_bclk, PinName i2s_mclk, PinName i2s_lrclk);
        NAU88C10(PinName i2c_sda, PinName i2c_scl, int i2c_addr, PinName i2s_tx, PinName i2s_rx, PinName i2s_bclk, PinName i2s_mclk, PinName i2s_lrclk, char i2s_master_enable, char codec_master_enable);

        // Config NAU88C10 codec through I2C
        void power(void);
        
        // setup audio format with sample rate, channel count and sampleBitLength
        void format(int sampleRate, char channelCount, char sampleBitLength);

        // Start audio recording
        void startRecord(void);

        // Read PCM data from codec
        void read(void);

        // Stop data transmit
        void stop(void);

        // attach callback function to process audio in/out
        void attach(void(*fptr)(void));
        void attach(Callback<void()> func);

        // start audio playing
        void startPlaying(void);
        
        // Write PCM data into codec
        void write(int *buffer, int from, int length);

        int  *rxBuffer;
                
    private:
        int  m_addr;
        char m_masterMode;
        
        I2C m_I2C;
        //I2S m_I2S;
        
        void I2C_WriteNAU88C10(uint8_t u8addr, uint16_t u16data);     
};

#endif
