/**
  ******************************************************************************
  * @file    nau88c10.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    22-February-2016
  * @brief   This file provides the nau88c10 Audio Codec driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
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
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "nau88c10.h"

/* Uncomment this line to enable verifying data sent to codec after each write 
   operation (for debug purpose) */
#if !defined (VERIFY_WRITTENDATA)  
/*#define VERIFY_WRITTENDATA*/
#endif /* VERIFY_WRITTENDATA */


/* Audio codec driver structure initialization */  
AUDIO_DrvTypeDef nau88c10_drv =
{
  nau88c10_Init,
  nau88c10_DeInit,
  nau88c10_ReadID,
  nau88c10_ReadRegister,
  nau88c10_WriteRegister,

  nau88c10_Play,
  nau88c10_Pause,
  nau88c10_Resume,
  nau88c10_Stop,

  nau88c10_SetVolume,
  nau88c10_SetMute,
  nau88c10_SetOutputMode,
  nau88c10_SetFrequency,

  nau88c10_Reset
};

static uint32_t outputEnabled = 0;
static uint32_t inputEnabled = 0;
/**
  * @}
  */ 

/** @defgroup nau88c10_Function_Prototypes
  * @{
  */
static uint8_t CODEC_IO_Write(uint8_t Addr, uint16_t Reg, uint16_t Value);
/**
  * @}
  */ 

/** @defgroup nau88c10_Private_Functions
  * @{
  */ 

/**
  * @brief Initializes the audio codec and the control interface.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param OutputInputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *  OUTPUT_DEVICE_BOTH, OUTPUT_DEVICE_AUTO, INPUT_DEVICE_DIGITAL_MICROPHONE_1,
  *  INPUT_DEVICE_DIGITAL_MICROPHONE_2, INPUT_DEVICE_DIGITAL_MIC1_MIC2, 
  *  INPUT_DEVICE_INPUT_LINE_1 or INPUT_DEVICE_INPUT_LINE_2.
  * @param Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param AudioFreq: Audio Frequency 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_Init(uint16_t DeviceAddr, uint16_t OutputInputDevice, uint32_t AudioFreq)
{
  //uint32_t counter = 0;
  //uint16_t output_device = OutputInputDevice & 0xFF;
  //uint16_t input_device = OutputInputDevice & 0xFF00;
  //uint16_t power_mgnt_reg_1 = 0;
  
  /*Power Management*/
  CODEC_IO_Write(DeviceAddr, 0x1, 0x015d);
  CODEC_IO_Write(DeviceAddr, 0x2, 0x0015);  // BSTEN, PGAEN and ADCEN
//  CODEC_IO_Write(DeviceAddr,0x3,0x0065);
  /*output in HeadPhone*/
  CODEC_IO_Write(DeviceAddr, 0x3, 0x00ED);
  /*Audio Control*/
  CODEC_IO_Write(DeviceAddr, 0x4, 0x0010);
  CODEC_IO_Write(DeviceAddr, 0x5, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0x6, 0x000c);
  CODEC_IO_Write(DeviceAddr, 0x7, 0x000a);
  CODEC_IO_Write(DeviceAddr, 0x8, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0x9, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0xa, 0x0008);
  CODEC_IO_Write(DeviceAddr, 0xb, 0x01FF); 
  CODEC_IO_Write(DeviceAddr, 0xc, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0xd, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0xe, 0x0108);
  CODEC_IO_Write(DeviceAddr, 0xf, 0x01FF);  // this is the ADC Gain control (ADCG)
  /*Equalizer*/
  CODEC_IO_Write(DeviceAddr, 0x12, 0x012c);
  CODEC_IO_Write(DeviceAddr, 0x13, 0x002c);
  CODEC_IO_Write(DeviceAddr, 0x14, 0x002c);
  CODEC_IO_Write(DeviceAddr, 0x15, 0x002c);
  CODEC_IO_Write(DeviceAddr, 0x16, 0x002c);
  /*DAC Limiter*/
  CODEC_IO_Write(DeviceAddr, 0x18, 0x0032);
  CODEC_IO_Write(DeviceAddr, 0x19, 0x0000);
  /*Notch Filter*/
  CODEC_IO_Write(DeviceAddr, 0x1b, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0x1c, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0x1d, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0x1e, 0x0000);
  /*ALC Control*/
  CODEC_IO_Write(DeviceAddr, 0x20, 0x0038);  // disabled, so this will control the Automatic Level Control, max=7, min=0.
  CODEC_IO_Write(DeviceAddr, 0x21, 0x000b);  // 0 for ALCHT means 0 hold time on gain, and B for ALCSL is -12 freq steps 
  CODEC_IO_Write(DeviceAddr, 0x22, 0x0032);  // bit 8 is low, so this is "Normal mode"
  CODEC_IO_Write(DeviceAddr, 0x23, 0x0000);
  /*PLL Control*/
  CODEC_IO_Write(DeviceAddr, 0x24, 0x0008);
  CODEC_IO_Write(DeviceAddr, 0x25, 0x000c);
  CODEC_IO_Write(DeviceAddr, 0x26, 0x0093);
  CODEC_IO_Write(DeviceAddr, 0x27, 0x00e9);
  /*BYP Control*/
  CODEC_IO_Write(DeviceAddr, 0x28, 0x0000);
  /*Input Output Mixer*/
  CODEC_IO_Write(DeviceAddr, 0x2c, 0x0003);
  CODEC_IO_Write(DeviceAddr, 0x2d, 0x0010);  // ADC is enabled, so gain comes from ADC block.
  CODEC_IO_Write(DeviceAddr, 0x2e, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0x2f, 0x0100);  // ADC Boost, PGABST is 1, and PMICBSTGAIN is 0
  CODEC_IO_Write(DeviceAddr, 0x30, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0x31, 0x0002);
  CODEC_IO_Write(DeviceAddr, 0x32, 0x0001);
  CODEC_IO_Write(DeviceAddr, 0x33, 0x0000);
  CODEC_IO_Write(DeviceAddr, 0x34, 0x0040);
  CODEC_IO_Write(DeviceAddr, 0x35, 0x0040);
  CODEC_IO_Write(DeviceAddr, 0x36, 0x00bF);
  CODEC_IO_Write(DeviceAddr, 0x37, 0x0040);
  /*output in HeadPhone*/
  CODEC_IO_Write(DeviceAddr, 0x38, 0x0001);
//  CODEC_IO_Write(DeviceAddr,0x38,0x0040);

  CODEC_IO_Write(DeviceAddr, 0x3C, 0x0004);

  return 0;
}

/**
  * @brief  Deinitializes the audio codec.
  * @param  None
  * @retval  None
  */
void nau88c10_DeInit(void)
{
  /* Deinitialize Audio Codec interface */
  AUDIO_IO_DeInit();
}

/**
  * @brief  Get the nau88c10 ID.
  * @param DeviceAddr: Device address on communication Bus.
  * @retval The nau88c10 ID
  */
uint32_t nau88c10_ReadID(uint16_t DeviceAddr)
{
  /* Initialize the Control interface of the Audio Codec */
  AUDIO_IO_Init();

  return nau88c10_ReadRegister(DeviceAddr, nau88c10_CHIPID_ADDR);
}

/**
  * @brief  Read the nau88c10 register.
  * @param DeviceAddr: Device address on communication Bus.
  * @param Reg: The register to read.
  * @retval The nau88c10 ID
  */
uint32_t nau88c10_ReadRegister(uint16_t DeviceAddr, uint16_t Reg)
{
  /* Initialize the Control interface of the Audio Codec */
  AUDIO_IO_Init();
  return (uint32_t)AUDIO_IO_Read(DeviceAddr, Reg);
}

uint32_t nau88c10_WriteRegister(uint16_t DeviceAddr, uint16_t reg, uint16_t value)
{
    AUDIO_IO_Init();
    CODEC_IO_Write(DeviceAddr, reg, value);
    return 0;
}

/**
  * @brief Start the audio Codec play feature.
  * @note For this codec no Play options are required.
  * @param DeviceAddr: Device address on communication Bus.   
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_Play(uint16_t DeviceAddr, uint16_t *pBuffer, uint16_t Size)
{
  uint32_t counter = 0;
 
  /* Resumes the audio file playing */  
  /* Unmute the output first */
  counter += nau88c10_SetMute(DeviceAddr, AUDIO_MUTE_OFF);
  
  return counter;
}

/**
  * @brief Pauses playing on the audio codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_Pause(uint16_t DeviceAddr)
{  
  uint32_t counter = 0;
 
  /* Pause the audio file playing */
  /* Mute the output first */
  counter += nau88c10_SetMute(DeviceAddr, AUDIO_MUTE_ON);
  
  /* Put the Codec in Power save mode */
  counter += CODEC_IO_Write(DeviceAddr, 0x02, 0x01);
 
  return counter;
}

/**
  * @brief Resumes playing on the audio codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_Resume(uint16_t DeviceAddr)
{
  uint32_t counter = 0;
 
  /* Resumes the audio file playing */  
  /* Unmute the output first */
  counter += nau88c10_SetMute(DeviceAddr, AUDIO_MUTE_OFF);
  
  return counter;
}

/**
  * @brief Stops audio Codec playing. It powers down the codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @param CodecPdwnMode: selects the  power down mode.
  *          - CODEC_PDWN_SW: only mutes the audio codec. When resuming from this 
  *                           mode the codec keeps the previous initialization
  *                           (no need to re-Initialize the codec registers).
  *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from this
  *                           mode, the codec is set to default configuration 
  *                           (user should re-Initialize the codec in order to 
  *                            play again the audio stream).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_Stop(uint16_t DeviceAddr, uint32_t CodecPdwnMode)
{
  uint32_t counter = 0;

  /* Mute the output first */
  counter += nau88c10_SetMute(DeviceAddr, AUDIO_MUTE_ON);

  if (CodecPdwnMode == CODEC_PDWN_SW)
  {
      /* Only output mute required*/
  }
  else /* CODEC_PDWN_HW */
  {
    /* Mute the AIF1 Timeslot 0 DAC1 path */
    counter += CODEC_IO_Write(DeviceAddr, 0x420, 0x0200);

    /* Mute the AIF1 Timeslot 1 DAC2 path */
    counter += CODEC_IO_Write(DeviceAddr, 0x422, 0x0200);

    /* Disable DAC1L_TO_HPOUT1L */
    counter += CODEC_IO_Write(DeviceAddr, 0x2D, 0x0000);

    /* Disable DAC1R_TO_HPOUT1R */
    counter += CODEC_IO_Write(DeviceAddr, 0x2E, 0x0000);

    /* Disable DAC1 and DAC2 */
    counter += CODEC_IO_Write(DeviceAddr, 0x05, 0x0000);

    /* Reset Codec by writing in 0x0000 address register */
    counter += CODEC_IO_Write(DeviceAddr, 0x0000, 0x0000);

    outputEnabled = 0;
  }

  return counter;
}

/**
  * @brief Sets higher or lower the codec volume level.
  * @param DeviceAddr: Device address on communication Bus.
  * @param Volume: a byte value from 0 to 255 (refer to codec registers 
  *         description for more details).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_SetVolume(uint16_t DeviceAddr, uint8_t Volume)
{
  uint32_t counter = 0;
  uint8_t convertedvol = VOLUME_CONVERT(Volume);

  counter = CODEC_IO_Write(DeviceAddr,0xb,convertedvol);

  return counter;
}

/**
  * @brief Enables or disables the mute feature on the audio codec.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
  *             mute mode.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_SetMute(uint16_t DeviceAddr, uint32_t Cmd)
{
  uint32_t counter = 0;
  
  /* Set the Mute mode */
  if(Cmd == AUDIO_MUTE_ON)
  {
    /* Soft Mute the AIF1 Timeslot 0 DAC1 path L&R */
    counter += CODEC_IO_Write(DeviceAddr, 0x36, 0x60);
  }
  else /* AUDIO_MUTE_OFF Disable the Mute */
  {
    /* Unmute the AIF1 Timeslot 0 DAC1 path L&R */
    counter += CODEC_IO_Write(DeviceAddr, 0x36, 0x5F);
  }

  return counter;
}

/**
  * @brief Switch dynamically (while audio file is played) the output target 
  *         (speaker or headphone).
  * @param DeviceAddr: Device address on communication Bus.
  * @param Output: specifies the audio output target: OUTPUT_DEVICE_SPEAKER,
  *         OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_SetOutputMode(uint16_t DeviceAddr, uint8_t Output)
{
  uint32_t counter = 0; 
  
  switch (Output) 
  {
  case OUTPUT_DEVICE_SPEAKER:
    /* Enable DAC1 (Left), Enable DAC1 (Right), 
    Disable DAC2 (Left), Disable DAC2 (Right)*/
    counter += CODEC_IO_Write(DeviceAddr, 0x05, 0x0C0C);
    
    /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x601, 0x0000);
    
    /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x602, 0x0000);
    
    /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x604, 0x0002);
    
    /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x605, 0x0002);
    break;
    
  case OUTPUT_DEVICE_HEADPHONE:
    /* Disable DAC1 (Left), Disable DAC1 (Right), 
    Enable DAC2 (Left), Enable DAC2 (Right)*/
    counter += CODEC_IO_Write(DeviceAddr, 0x05, 0x0303);
    
    /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x601, 0x0001);
    
    /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x602, 0x0001);
    
    /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x604, 0x0000);
    
    /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x605, 0x0000);
    break;
    
  case OUTPUT_DEVICE_BOTH:
    /* Enable DAC1 (Left), Enable DAC1 (Right), 
    also Enable DAC2 (Left), Enable DAC2 (Right)*/
    counter += CODEC_IO_Write(DeviceAddr, 0x05, 0x0303 | 0x0C0C);
    
    /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x601, 0x0001);
    
    /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x602, 0x0001);
    
    /* Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x604, 0x0002);
    
    /* Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x605, 0x0002);
    break;
    
  default:
    /* Disable DAC1 (Left), Disable DAC1 (Right), 
    Enable DAC2 (Left), Enable DAC2 (Right)*/
    counter += CODEC_IO_Write(DeviceAddr, 0x05, 0x0303);
    
    /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x601, 0x0001);
    
    /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x602, 0x0001);
    
    /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x604, 0x0000);
    
    /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
    counter += CODEC_IO_Write(DeviceAddr, 0x605, 0x0000);
    break;    
  }  
  return counter;
}

/**
  * @brief Sets new frequency.
  * @param DeviceAddr: Device address on communication Bus.
  * @param AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_SetFrequency(uint16_t DeviceAddr, uint32_t AudioFreq)
{
  uint32_t counter = 0;
 
  /*  Clock Configurations */
  switch (AudioFreq)
  {
  case  AUDIO_FREQUENCY_8K:
    /* AIF1 Sample Rate = 8 (KHz), ratio=256 */ 
    counter += CODEC_IO_Write(DeviceAddr, 0x210, 0x0003);
    break;
    
  case  AUDIO_FREQUENCY_16K:
    /* AIF1 Sample Rate = 16 (KHz), ratio=256 */ 
    counter += CODEC_IO_Write(DeviceAddr, 0x210, 0x0033);
    break;
    
  case  AUDIO_FREQUENCY_48K:
    /* AIF1 Sample Rate = 48 (KHz), ratio=256 */ 
    counter += CODEC_IO_Write(DeviceAddr, 0x210, 0x0083);
    break;
    
  case  AUDIO_FREQUENCY_96K:
    /* AIF1 Sample Rate = 96 (KHz), ratio=256 */ 
    counter += CODEC_IO_Write(DeviceAddr, 0x210, 0x00A3);
    break;
    
  case  AUDIO_FREQUENCY_11K:
    /* AIF1 Sample Rate = 11.025 (KHz), ratio=256 */ 
    counter += CODEC_IO_Write(DeviceAddr, 0x210, 0x0013);
    break;
    
  case  AUDIO_FREQUENCY_22K:
    /* AIF1 Sample Rate = 22.050 (KHz), ratio=256 */ 
    counter += CODEC_IO_Write(DeviceAddr, 0x210, 0x0043);
    break;
    
  case  AUDIO_FREQUENCY_44K:
    /* AIF1 Sample Rate = 44.1 (KHz), ratio=256 */ 
    counter += CODEC_IO_Write(DeviceAddr, 0x210, 0x0073);
    break; 
    
  default:
    /* AIF1 Sample Rate = 48 (KHz), ratio=256 */ 
    counter += CODEC_IO_Write(DeviceAddr, 0x210, 0x0083);
    break; 
  }
  return counter;
}

/**
  * @brief Resets nau88c10 registers.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t nau88c10_Reset(uint16_t DeviceAddr)
{
  uint32_t counter = 0;
  
  /* Reset Codec by writing in 0x0000 address register */
  counter = CODEC_IO_Write(DeviceAddr, 0x0000, 0x0000);
  outputEnabled = 0;
  inputEnabled=0;

  return counter;
}

/**
  * @brief  Writes/Read a single data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address 
  * @param  Value: Data to be written
  * @retval None
  */
static uint8_t CODEC_IO_Write(uint8_t Addr, uint16_t Reg, uint16_t Value)
{
  uint32_t result = 0;
  
 AUDIO_IO_Write(Addr, Reg, Value);
  
#ifdef VERIFY_WRITTENDATA
  /* Verify that the data has been correctly written */
  result = (AUDIO_IO_Read(Addr, Reg) == Value)? 0:1;
#endif /* VERIFY_WRITTENDATA */
  
  return result;
}
