// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "AudioClass.h"
#include <stdint.h>
#include <stdlib.h>

#include "nau88c10.h"
#include "stm32f4xx_hal.h"
#include "stm32412g_discovery.h"
#include "stm32412g_discovery_audio.h"

static uint32_t _sampleRate;
static uint16_t _sampleBitDepth;
static uint8_t _channels;
static uint8_t _durationInSeconds;

static uint16_t _rx_buffer[BATCH_TRANSMIT_SIZE];
static uint16_t _tx_buffer[BATCH_TRANSMIT_SIZE];

static char * _wavFile;
static char * _recordCursor;
static uint16_t * _playCursor;
static int _audioFileSize;
static int _audioRemSize;

static volatile char _flag = 0;
static AUDIO_STATE_TypeDef _audioState;

AudioClass::AudioClass()
{
    format(DEFAULT_SAMPLE_RATE, DEFAULT_BITS_PER_SAMPLE);
}

/* 
 * @brief Configure the audio data format
*/
void AudioClass::format(unsigned int sampleRate, unsigned short sampleBitLength)
{
    _sampleRate = sampleRate;
    _sampleBitDepth = sampleBitLength;
    _channels = STEREO;

    // Currently we ONLY support 16 bit depth audio sample
    unsigned short bitDepth;
    if (sampleBitLength == 16)
    {
        bitDepth = I2S_DATAFORMAT_16B;
    }
    else if (sampleBitLength == 24)
    {
        // TODO
        bitDepth = I2S_DATAFORMAT_24B;
    }
    else if (sampleBitLength == 32)
    {
        // TODO
        bitDepth = I2S_DATAFORMAT_32B;
    }
    else
    {
        bitDepth = I2S_DATAFORMAT_16B;
    }

    BSP_AUDIO_IN_OUT_Init(OUTPUT_DEVICE_AUTO, bitDepth, sampleRate);

    // Set audio initialization state
    _audioState = AUDIO_STATE_INIT;
}

int AudioClass::start(uint16_t *transmitBuf, uint16_t *readBuf, unsigned int size)
{
    if (transmitBuf == NULL || readBuf == NULL)
    {
        return AUDIO_ERROR;
    }

    BSP_AUDIO_In_Out_Transfer(transmitBuf, readBuf, size);
    return AUDIO_OK;
}

/*
** @brief Start recording audio data using underlying codec
** @param Pointer to audio file
** @param fileSize Size of audio file
** @param durationInSeconds Audio duration to be recorded
** @retval AUDIO_OK if correct recording, else wrong recording
*/
int AudioClass::startRecord(char *audioFile, int fileSize, int durationInSeconds)
{
    if ((audioFile == NULL) || (fileSize < WAVE_HEADER_SIZE) || (durationInSeconds <= 0)) {
        return AUDIO_ERROR;
    }

    _flag = 0;
    _wavFile = audioFile;
    _audioFileSize = fileSize;
    _durationInSeconds = durationInSeconds;
    _recordCursor = _wavFile + WAVE_HEADER_SIZE;

    // Calculate the pcm data size to be recorded per given duration
    int pcmAudioSize = _sampleRate * _channels * _sampleBitDepth * _durationInSeconds / 8;
    if (pcmAudioSize < _audioFileSize - WAVE_HEADER_SIZE)
    {
        _audioFileSize = pcmAudioSize + WAVE_HEADER_SIZE;
    }

    _audioState = AUDIO_STATE_RECORDING;
    memset(_tx_buffer, 0x0, BATCH_TRANSMIT_SIZE*2);
    start(_tx_buffer, _rx_buffer, BATCH_TRANSMIT_SIZE);

    return AUDIO_OK;
}

/*
** @brief Start playing audio data using underlying codec
** @param audioFile Pointer to audio file
** @param fileSize Size of audio file
** @param channels Number of channels of the audio file
** @retval AUDIO_OK if correct playing, else wrong playing
*/
int AudioClass::startPlay(char *audioFile, int fileSize)
{
    if (audioFile == NULL || fileSize <= WAVE_HEADER_SIZE)
    {
        return AUDIO_ERROR;
    }

    _wavFile = audioFile;
    _audioFileSize = fileSize;

    // Set the current audio pointer position
    _playCursor = (uint16_t *)(_wavFile + WAVE_HEADER_SIZE);

    // Set the total number of data to be played (count in half-word)
    int pcmDataSize = (fileSize - WAVE_HEADER_SIZE) / STEREO;

    // Start the audio player
    _audioState = AUDIO_STATE_PLAYING;
    BSP_AUDIO_OUT_Play((uint16_t *)_playCursor, (uint32_t)(fileSize - WAVE_HEADER_SIZE));

    _audioRemSize = pcmDataSize - DMA_MAX(pcmDataSize);
    _playCursor += DMA_MAX(pcmDataSize);

    return AUDIO_OK;
}

/*
 * @brief stop audio data transmition
*/
void AudioClass::stop()
{
    BSP_AUDIO_STOP();
    _audioState = AUDIO_STATE_IDLE;
}

int AudioClass::getAudioState()
{
    return _audioState;
}

/*
 * @brief compose the WAVE header according to the raw data size
 */
void AudioClass::genericWAVHeader(WaveHeader *hdr, int pcmDataSize, uint32_t sampleRate, uint16_t sampleBitDepth, uint8_t channels)
{
    if (hdr == NULL)
    {
        return;
    }

    memcpy(&hdr->RIFF_marker, "RIFF", 4);
    memcpy(&hdr->filetype_header, "WAVE", 4);
    memcpy(&hdr->format_marker, "fmt ", 4);
    hdr->data_header_length = 16;
    hdr->format_type = 1;
    hdr->number_of_channels = channels;
    hdr->sample_rate = sampleRate;
    hdr->bytes_per_second = sampleRate * channels * sampleBitDepth / 8;
    hdr->bytes_per_frame = channels * sampleBitDepth / 8;
    hdr->bits_per_sample = sampleBitDepth;
    hdr->file_size = pcmDataSize + 36;
    memcpy(&hdr->data_chunk_id, "data", 4);
    hdr->data_chunk_size = pcmDataSize;
}

/*
** Get wave file
*/
char *AudioClass::getWav(int *file_size)
{
    if (_audioState == AUDIO_STATE_RECORDING)
    {
        _audioState = AUDIO_STATE_RECORDING_FINISH;
    }

    int currentSize = _recordCursor - _wavFile;
    *file_size = (int)currentSize;

    // write wave header for this audio file
    WaveHeader hdr;
    genericWAVHeader(&hdr, currentSize - WAVE_HEADER_SIZE, _sampleRate, _sampleBitDepth, _channels);
    memcpy(_wavFile, &hdr, sizeof(WaveHeader));

    return _wavFile;
}

int AudioClass::convertToMono(char *audioFile, int size, int sampleBitLength)
{
    if (sampleBitLength != 16 && sampleBitLength != 24 && sampleBitLength != 32)
    {
        // TODO: log error
        return -1;
    }

    if (audioFile == NULL || size < WAVE_HEADER_SIZE)
    {
        // TODO: log error
        return -1;
    }

    int bytesPerSample = sampleBitLength / 8;
    int curFileSize = 0;

    char *curReader = audioFile + WAVE_HEADER_SIZE + bytesPerSample * 2;
    char *curWriter = audioFile + WAVE_HEADER_SIZE + bytesPerSample;

    if (sampleBitLength == 16)
    {
        while (curReader < audioFile + size)
        {
            *(uint16_t *)curWriter = *((uint16_t *)curReader);

            curWriter += bytesPerSample;
            curReader += bytesPerSample * 2;
        }
    }
    else if (sampleBitLength == 32)
    {
        while (curReader < audioFile + size)
        {
            *(uint32_t *)curWriter = *((uint32_t *)curReader);

            curWriter += bytesPerSample;
            curReader += bytesPerSample * 2;
        }
    }
    else
    {
        while (curReader < audioFile + size)
        {
            memcpy(curWriter, curReader, bytesPerSample);

            curWriter += bytesPerSample;
            curReader += bytesPerSample * 2;
        }
    }

    curFileSize = curWriter - audioFile;

    // re-calculate wave header since the raw data is re-sized from stereo to mono
    WaveHeader hdr;
    genericWAVHeader(&hdr, curFileSize - WAVE_HEADER_SIZE, _sampleRate, _sampleBitDepth, 1);
    memcpy(audioFile, &hdr, sizeof(WaveHeader));

    // clean up the remaining file
    //memset(curWriter, 0, audioFile + size - curWriter);

    return curFileSize;
}

double AudioClass::getRecordedDuration()
{
    if (_wavFile == NULL)
    {
        return 0;
    }

    int pcmDataSize = _recordCursor - _wavFile - WAVE_HEADER_SIZE;

    int bytes_per_second = _sampleRate * _channels * _sampleBitDepth / 8;
    return pcmDataSize / bytes_per_second;
    ;
}

int AudioClass::getCurrentSize()
{
    if (_wavFile == NULL)
    {
        return 0;
    }

    return _recordCursor - _wavFile;
}

/*------------------------------------------------------------------------------
       Callbacks implementation:
           the callbacks API are defined __weak in the stm32412g_discovery_audio.c file
           and their implementation should be done the user code if they are needed.
           Below some examples of callback implementations.
  ----------------------------------------------------------------------------*/
/**
  * @brief  Manages the full Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
    if (_audioState == AUDIO_STATE_RECORDING)
    {
        char *bufferTail = _wavFile + _audioFileSize;
        if (_recordCursor >= bufferTail)
        {
            _audioState = AUDIO_STATE_RECORDING_FINISH;
            return;
        }

        int copySize = 0;
        if (_recordCursor + BATCH_TRANSMIT_SIZE * 2 > bufferTail)
        {
            copySize = bufferTail - _recordCursor;
        }
        else
        {
            copySize = BATCH_TRANSMIT_SIZE * 2;
        }

        // Copy audio data from codec to user buffer.
        if (_flag == 0)
        {
            memcpy(_recordCursor, (char *)(_rx_buffer), copySize);
            _recordCursor += copySize;

            memset(_rx_buffer, 0x0, BATCH_TRANSMIT_SIZE * 2);
            BSP_AUDIO_In_Out_Transfer(_rx_buffer, _tx_buffer, BATCH_TRANSMIT_SIZE);
            _flag = 1;
        }
        else
        {
            memcpy(_recordCursor, (char *)(_tx_buffer), copySize);
            _recordCursor += copySize;

            memset(_tx_buffer, 0x0, BATCH_TRANSMIT_SIZE * 2);
            BSP_AUDIO_In_Out_Transfer(_tx_buffer, _rx_buffer, BATCH_TRANSMIT_SIZE);
            _flag = 0;
        }
    }
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    if (_audioState == AUDIO_STATE_PLAYING)
    {
        /* Calculate the remaining audio data in the file and the new size
        for the DMA transfer. If there is no audio data to be transferred,
        just stop playing */

        // Check if the end of file has been reached
        uint16_t *bufferTail = (uint16_t *)(_wavFile + _audioFileSize);
        int remainingSize = (bufferTail - _playCursor) / STEREO;

        if (_audioRemSize > 0)
        {
            // Replay from the current position
            BSP_AUDIO_OUT_ChangeBuffer((uint16_t *)_playCursor, DMA_MAX(_audioRemSize));

            // Update the current pointer position
            _playCursor += DMA_MAX(_audioRemSize);

            _audioRemSize -= DMA_MAX(_audioRemSize);
        }
        else
        {
            _audioState = AUDIO_STATE_PLAYING_FINISH;
            return;
        }
    }
}

/**
  * @brief  Manages the DMA FIFO error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
    /* Display message on the LCD screen */
}