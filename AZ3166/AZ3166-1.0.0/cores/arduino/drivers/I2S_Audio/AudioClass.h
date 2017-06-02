/**
 ******************************************************************************
 * The MIT License (MIT)
 * Copyright (C) 2017 Microsoft Corp. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "mbed.h"
#include "NAU88C10.h"

#define DURATION_IN_SECONDS         2
#define DEFAULT_SAMPLE_RATE         8000
#define DEFAULT_BITS_PER_SAMPLE     16
#define MONO                        1
#define STEREO                      2
#define WAVE_HEADER_SIZE            44          // 44 bytes
#define BATCH_TRANSMIT_SIZE         (1*1024)    // 2048 bytes

typedef struct
{
    char RIFF_marker[4];
    uint32_t file_size;
    char filetype_header[4];
    char format_marker[4];
    uint32_t data_header_length;
    uint16_t format_type;
    uint16_t number_of_channels;
    uint32_t sample_rate;
    uint32_t bytes_per_second;
    uint16_t bytes_per_frame;
    uint16_t bits_per_sample;
    char data_chunck_id[4];
    uint32_t data_chunck_size;
} WaveHeader;

class AudioClass {
    public:
        AudioClass();
        void format(unsigned int sampleRate = DEFAULT_SAMPLE_RATE, unsigned short sampleBitLength = DEFAULT_BITS_PER_SAMPLE);    
        void startRecord(char * audioFile, int fileSize, int durationInSeconds);
        bool recordComplete();
        char* getWav(int *fileSize);
        double getRecordedDuration();
        int getCurrentSize();
        void stop();
        int convertToMono(char * audioFile, int size, int sampleBitLength);

    private:
        void start(uint16_t * recordBuf, uint16_t * playBuf, unsigned int size);
        void genericWAVHeader(WaveHeader* header, int pcmDataSize, uint32_t sampleRate, uint16_t sampleBitDepth, uint8_t channels);
};

#endif
