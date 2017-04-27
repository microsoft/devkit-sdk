#include "AudioClass.h"
#include <stdint.h>
#include <stdlib.h>

#include "stlogo.h"
#include "nau88c10.h"
#include "stm32f4xx_hal.h"
#include "stm32412g_discovery.h"
#include "stm32412g_discovery_audio.h"

static uint32_t m_sample_rate;
static uint8_t m_bit_depth;
static uint8_t m_channels;
static uint8_t m_duration;

static uint16_t m_rx_buffer[BATCH_TRANSMIT_SIZE];
static uint16_t m_tx_buffer[BATCH_TRANSMIT_SIZE];

static char * m_wavFile;
static char * m_record_cursor;
static int m_file_size;
static int m_max_pcm_size;

static volatile char flag = 0;
static bool record_finish = false;


AudioClass::AudioClass()
{
    format(DEFAULT_SAMPLE_RATE, DEFAULT_BITS_PER_SAMPLE);
}

/* 
 * @brief Configure the audio data format
*/
void AudioClass::format(uint32_t sampleRate, uint8_t sampleBitLength)
{
    m_sample_rate = sampleRate;
    m_channels = STEREO;
    m_bit_depth = sampleBitLength;

    // Currently we ONLY support 16 bit depth audio sample
    uint32_t sample_bit_depth;
    if (m_bit_depth == 16) {
        sample_bit_depth = I2S_DATAFORMAT_16B;
    } else if (m_bit_depth == 24){
        // TODO
        sample_bit_depth = I2S_DATAFORMAT_24B;
    } else if (m_bit_depth == 32) {
        // TODO
        sample_bit_depth = I2S_DATAFORMAT_32B;
    } else {
        sample_bit_depth = I2S_DATAFORMAT_16B;
    }
    
    BSP_AUDIO_IN_OUT_Init(OUTPUT_DEVICE_AUTO, sample_bit_depth, sampleRate);
    record_finish = false;
}


void AudioClass::start(uint16_t * transmitBuf, uint16_t * readBuf, uint32_t size)
{
    if (transmitBuf == NULL || readBuf == NULL) {
        return;
    }

    printf("Start recording...\r\n");
    BSP_AUDIO_In_Out_Transfer(transmitBuf, readBuf, size);
}

/*
** @brief Start recording audio data usine underlying codec
*/
void AudioClass::startRecord(char * audioFile, int fileSize, uint8_t durationInSeconds)
{
    if (audioFile == NULL) return;

    if (fileSize < WAVE_HEADER_SIZE) return;

    record_finish = false;
    m_wavFile = audioFile;
    m_file_size = fileSize;
    m_duration = durationInSeconds;
    m_record_cursor = m_wavFile + WAVE_HEADER_SIZE;

    // Calculate the max pcm data size according to the given duration
    m_max_pcm_size = m_sample_rate * m_channels * m_bit_depth * m_duration / 8;
    if (m_max_pcm_size > m_file_size - WAVE_HEADER_SIZE) {
        m_max_pcm_size = m_file_size - WAVE_HEADER_SIZE;
    }

    printf("Max audio data size(include 44 bytes header): %d\r\n", m_max_pcm_size + WAVE_HEADER_SIZE);
    start(m_tx_buffer, m_rx_buffer, BATCH_TRANSMIT_SIZE);
}

/*
 * @brief stop audio data transmition
*/
void AudioClass::stop()
{
    printf("Stop recording.\r\n");
    BSP_AUDIO_STOP();
    record_finish = true;
}

/*
 * @brief query the record status to check if the DMA transmition is completed
 */   
bool AudioClass::recordComplete() 
{
    return record_finish;
}

/*
 * @brief compose the WAVE header according to the raw data size
 */
void AudioClass::genericWAVHeader(WaveHeader* hdr, int pcmDataSize, uint32_t sampleRate, int sampleBitDepth, uint8_t channels)
{
    if (hdr == NULL) {
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
    memcpy(&hdr->data_chunck_id, "data", 4);
    hdr->data_chunck_size = pcmDataSize;
}

/*
** Get wave file
*/
char * AudioClass::getWav(int *file_size)
{
    int currentSize = m_record_cursor - m_wavFile;
    *file_size  = (int)currentSize;

    // write wave header for this audio file
    WaveHeader hdr;
    genericWAVHeader(&hdr, currentSize - WAVE_HEADER_SIZE, m_sample_rate, m_bit_depth, m_channels);
    memcpy(m_wavFile, &hdr, sizeof(WaveHeader));

    return m_wavFile;
}

int AudioClass::convertToMono(char * audioFile, int size, uint8_t sampleBitLength)
{
    if (sampleBitLength != 16 && sampleBitLength != 24 && sampleBitLength != 32) {
        // TODO: log error
        return -1;
    }

    if (audioFile == NULL || size < WAVE_HEADER_SIZE) {
        // TODO: log error
        return -1;
    }

    int bytesPerSample = sampleBitLength / 8;
    int curFileSize = 0;

    char * curReader = audioFile + WAVE_HEADER_SIZE + bytesPerSample * 2;
    char * curWriter = audioFile + WAVE_HEADER_SIZE + bytesPerSample;

    while (curReader < audioFile + size) {
        if (sampleBitLength == 16) {
            *(uint16_t *)curWriter = *((uint16_t *)curReader);
        } else if (sampleBitLength == 32) {
            *(uint32_t *)curWriter = *((uint32_t *)curReader);
        } else {
            memcpy(curWriter, curReader, bytesPerSample);
        }

        curWriter += bytesPerSample;
        curReader += bytesPerSample * 2;
    }

    curFileSize = curWriter - audioFile;

    // re-calculate wave header since the raw data is re-sized from stereo to mono
    WaveHeader hdr;
    genericWAVHeader(&hdr, curFileSize - WAVE_HEADER_SIZE, m_sample_rate, m_bit_depth, 1);
    memcpy(audioFile, &hdr, sizeof(WaveHeader));

    // clean up the remaining file
    //memset(curWriter, 0, audioFile + size - curWriter);

    return curFileSize;
}

double AudioClass::getRecordedDuration()
{
    if (m_wavFile == NULL) {
        return 0;
    }

    int pcmDataSize = m_record_cursor - m_wavFile - WAVE_HEADER_SIZE;  

    int bytes_per_second = m_sample_rate * m_channels * m_bit_depth / 8;
    return pcmDataSize / bytes_per_second;;
}

int AudioClass::getCurrentSize()
{
    if (m_wavFile == NULL) {
        return 0;
    }
    
    return m_record_cursor - m_wavFile;
}

AudioClass Audio;

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
    char * bufferTail = m_wavFile + m_max_pcm_size + WAVE_HEADER_SIZE;

    if (m_record_cursor >=  bufferTail) {
        record_finish = true;
        return;
    }

    int copySize = 0;
    if (m_record_cursor + BATCH_TRANSMIT_SIZE * 2 > bufferTail) {
        copySize = bufferTail - m_record_cursor;
    } else {
        copySize = BATCH_TRANSMIT_SIZE * 2;
    }

    // Copy audio data from codec to user buffer.
    if(flag == 0)
    {
        memcpy(m_record_cursor, (char *)(m_rx_buffer), copySize);
        m_record_cursor += copySize;

        BSP_AUDIO_In_Out_Transfer(m_rx_buffer, m_tx_buffer, BATCH_TRANSMIT_SIZE);
        flag = 1;
    }
    else
    {
        memcpy(m_record_cursor, (char *)(m_tx_buffer), copySize);
        m_record_cursor += copySize;

        BSP_AUDIO_In_Out_Transfer(m_tx_buffer, m_rx_buffer, BATCH_TRANSMIT_SIZE);
        flag = 0;
    }
}

/**
  * @brief  Manages the full Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
}

/**
  * @brief  Manages the DMA FIFO error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
  /* Display message on the LCD screen */
    printf("dma error\r\n");
}