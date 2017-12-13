// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __AUDIO_CLASS_V2_H__
#define __AUDIO_CLASS_V2_H__

#include "mbed.h"
#include "NAU88C10.h"

#define DURATION_IN_SECONDS         2
#define DEFAULT_SAMPLE_RATE         8000
#define DEFAULT_BITS_PER_SAMPLE     16
#define MONO                        1
#define STEREO                      2
#define WAVE_HEADER_SIZE            44         // 44 bytes
#define AUDIO_CHUNK_SIZE            512        // 512 bytes

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
    char data_chunk_id[4];
    uint32_t data_chunk_size;
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

typedef void (*callbackFunc)();

class AudioClass {
    public:
        // Singleton class:
        // This is creation point for static instance variable
        static AudioClass& getInstance()
        {
            // Single audio instance
            static AudioClass audioInstance;
            return audioInstance;
        }
        // Configure the audio data format.
        void format(unsigned int sampleRate = DEFAULT_SAMPLE_RATE, unsigned short sampleBitLength = DEFAULT_BITS_PER_SAMPLE);
        // Stop audio data transmition.
        void stop();
        // Get status of the audio driver. Please use this API to query whether the playing/recording process is completed.
        int getAudioState();

        // Callback scenario methods :
        // Start recording audio data and call func after every 512 bytes buffer was recorded.
        int startRecord(callbackFunc func = NULL);
        // Start playing audio data and call func after every 512 bytes buffer was played.
        int startPlay(callbackFunc func = NULL);
        // Read recorded data from buffer inside AudioClass to given buffer.
        int readFromRecordBuffer(char* buffer, int length);
        // Write played data from given buffer to buffer inside AudioClass.
        int writeToPlayBuffer(char* buffer, int length);

        // WAV format data scenario methods :
        // Start recording audio data and save a WAV format data to audioBuffer.
        int startRecord(char* audioBuffer, int size);
        // Start playing WAV format data in audioBuffer.
        int startPlay(char* audioBuffer, int size);
        // Get current recorded or played WAV format data size in byte.
        int getCurrentSize();
        // Convert the given stereo WAV format data to mono WAV format data.
        int convertToMono(char* audioBuffer, int size, int sampleBitLength);

    private:
        void genericWAVHeader(WaveHeader* header, int pcmDataSize, uint32_t sampleRate, uint16_t sampleBitDepth, uint8_t channels);

        /* Private constructor to prevent instancing */
        AudioClass();
};

#endif
