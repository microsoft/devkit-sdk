// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __AUDIO_CLASS_V2_H__
#define __AUDIO_CLASS_V2_H__

#include "mbed.h"
#include "nau88c10.h"

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

        /**
         * @brief   Configures the audio driver.
         * @param   sampleRate:              audio sample rate.
         *          sampleBitLength:         bit length of the audio sample.
         */
        void format(unsigned int sampleRate = DEFAULT_SAMPLE_RATE, unsigned short sampleBitLength = DEFAULT_BITS_PER_SAMPLE);

        /**
         * @brief  Stop audio data transmition.
         */
        void stop();

        /**
         * @brief   Get current state of the audio driver. This API can be used to query whether the playing/recording process is completed.
         * 
         * @returns enum value of AUDIO_STATE_TypeDef.
         */
        int getAudioState();

        // Audio record/play callback methods:

        /**
         * @brief   Start recording audio data, the attached callback function will be invoked when each DMA transfer is completed.
         * 
         * @param   func:                   function to be called when each audio chunk (size is defined with AUDIO_CHUNK_SIZE) is recorded.
         *                                  e.g. user can use copy the recorded audio data out to the application buffer.
         * 
         * @returns 0 (AUDIO_OK) if success, error code otherwise.
         */
        int startRecord(callbackFunc func = NULL);
        
        /**
         * @brief   Start playing audio data, the attached callback function will be invoked when each DMA transfer is completed.
         * 
         * @param   func:                   function to be called when each audio chunk (size is defined with AUDIO_CHUNK_SIZE) is played.
         *                                  e.g. user can use copy the new audio data to the internal play buffer.
         * @returns 0 (AUDIO_OK) if success, error code otherwise.
         */
        int startPlay(callbackFunc func = NULL);
        
        /**
         * @brief   Read recorded data from internal driver buffer to user application buffer.
         * 
         * @param   buffer:                 user buffer to save the recorded audio data.
         *          length:                 size of the user buffer in bytes.
         *
         * @returns 0 (AUDIO_OK) if success, error code otherwise.
         */
        int readFromRecordBuffer(char* buffer, int length);

        /**
         * @brief   write the audio data to be played from user application buffer to the internal driver buffer.
         * 
         * @param   buffer:                 user buffer has the audio data to be played.
         *          length:                 size of the user buffer in bytes.
         *
         * @returns 0 (AUDIO_OK) if success, error code otherwise.
         */
        int writeToPlayBuffer(char* buffer, int length);

        // Record/play wav audio directly:

        /**
         * @brief   Start recording audio data and save a WAV format data to user buffer.
         * 
         * @param   buffer:                 user buffer to save the recorded audio data.
         *          length:                 size of the user buffer in bytes.
         *
         * @returns size of the recorded audio in bytes.
         */
        int startRecord(char* audioBuffer, int size);
        
        /**
         * @brief  Start playing WAV format data in audioBuffer.
         * 
         * @param   buffer:                 user buffer to save the recorded audio data.
         *          length:                 size of the user buffer in bytes.
         *
         * @returns 0 (AUDIO_OK) if success, error code otherwise.
         */
        int startPlay(char* audioBuffer, int size);

        /**
         * @brief  Get current recorded or played WAV format data size in byte.
         *
         * @returns size of the current audio size.
         */
        int getCurrentSize();
        
        /**
         * @brief  Convert the given stereo WAV format data to mono WAV format data.
         *
         * @param   buffer:                 user buffer to save the recorded audio data.
         *          size:                   size of stereo audio in the buffer.
         *          sampleBitLength:        bit length of the audio sample.
         * 
         * @returns size of the mono audio size.
         */
        int convertToMono(char* audioBuffer, int size, int sampleBitLength);

        /**
         * @brief  Controls the current audio volume level.
         * 
         * @param  volume       Volume level to be set in percentage from 0% to 100% (0 for Mute and 100 for Max volume level).
         * 
         * @returns true on success, or false on failure.
         */
        bool setVolume(uint8_t volume);

        /**
         * @brief   Read the given nau88c10 register
         * 
         * @param  registerAddress       Register address.
         * 
         * @returns the integer value in the register.
         */        
        uint16_t readRegister(uint16_t registerAddress);
        
        /**
         * @brief   write the given nau88c10 register
         * 
         * @param  registerAddress       Register address.
         * @param  value                 The value to write.
         * 
         */        
        void writeRegister(uint16_t registerAddress, uint16_t value);

        /**
         * @brief   enable automatic level control with given min and max gain as per ALCMXGAIN and ALCMNGAIN (register 0x20)
         * 
         * @param  maxGain       A value between 0 and 7.
         * @param  minGain       A value between 0 and 7
         * 
         */        
        void enableLevelControl(uint8_t maxGain, uint8_t minGain); 

        /**
         * @brief   disable automatic level control (register 0x20)
         * 
         */
        void disableLevelControl();

        /**
         * @brief   set the Programmable Gain Amplifier directly (this will disable automatic level control)
         * 
         * @param  gain       A value between 0 and 0x3F.
         */
        void setPGAGain(uint8_t gain);

    private:
        void genericWAVHeader(WaveHeader* header, int pcmDataSize, uint32_t sampleRate, uint16_t sampleBitDepth, uint8_t channels);

        /* Private constructor to prevent instancing */
        AudioClass();
};

#endif
