// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

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

typedef enum 
{
  AUDIO_STATE_IDLE = 0,
  AUDIO_STATE_INIT,
  AUDIO_STATE_RECORDING,
  AUDIO_STATE_PLAYING,
  AUDIO_STATE_RECORDING_FINISH,
  AUDIO_STATE_PLAYING_FINISH
} AUDIO_STATE_TypeDef;

class AudioClass {
    public:
        void format(unsigned int sampleRate = DEFAULT_SAMPLE_RATE, unsigned short sampleBitLength = DEFAULT_BITS_PER_SAMPLE);    
        void startRecord(char * audioFile, int fileSize, int durationInSeconds);
        char* getWav(int *fileSize);
        double getRecordedDuration();
        int getCurrentSize();
        void stop();
        int convertToMono(char * audioFile, int size, int sampleBitLength);
        void startPlay(char * audioFile, int size);
        int getAudioState();

        // Singleton class:
        // This is creation point for static instance variable
        static AudioClass& getInstance()
        {
            // Single audio instance
            static AudioClass audioInstance;
            return audioInstance;
        }

    private:
        void start(uint16_t * recordBuf, uint16_t * playBuf, unsigned int size);
        void genericWAVHeader(WaveHeader* header, int pcmDataSize, uint32_t sampleRate, uint16_t sampleBitDepth, uint8_t channels);

        /* Private constructor to prevent instancing */
        AudioClass();
};

#endif
