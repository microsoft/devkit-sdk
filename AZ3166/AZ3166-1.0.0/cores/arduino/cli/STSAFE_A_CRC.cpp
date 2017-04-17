/*
 * \file STSAFE_A_CRC.cpp
 * \brief STSAFE-A CRC routine.
 * \date 08/08/2016
 * \author Christophe Delaunay
 * \version 1.1
 * \copyright 2016 STMicroelectronics
 *********************************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
 *
 * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/myliberty
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
 * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *********************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "STSAFE_A.h"

#define POLY ((uint16_t)0x1021)

/************************ crc16table ************************/
static uint16_t* Crc16CcittTable = NULL;

static void crc16_ccitt_table_init(void)
{
  uint16_t crc, c;

  Crc16CcittTable = (uint16_t*)malloc(256 * sizeof(crc));

  for(unsigned i(0); i < 256; ++i)
  {
    crc = 0;
    c = i << 8;
    for(unsigned j(0); j < 8; ++j)
    {
      if(((crc ^ c) & 0x8000) != 0)
      {
        crc = (uint16_t)((crc << 1) ^ POLY);
      }
      else
      {
        crc <<= 1;
      }
      c <<= 1;
    }

    Crc16CcittTable[i] = crc;
  }
}


/************************ crc16_ccitt ************************/
static uint8_t SwapBitsInByte(uint8_t byte)
{
  uint8_t temp = 0;

  /* Swap bits in each Byte */
  temp |= (byte & 0x80)>>7;
  temp |= (byte & 0x40)>>5;
  temp |= (byte & 0x20)>>3;
  temp |= (byte & 0x10)>>1;
  temp |= (byte & 0x08)<<1;
  temp |= (byte & 0x04)<<3;
  temp |= (byte & 0x02)<<5;
  temp |= (byte & 0x01)<<7;
  return temp;
}

/************************ crc16_ccitt ************************/
static uint16_t crc16_ccitt(DataExchangeBuffer* data_buffer)
{
  uint16_t ndx, crc = 0xFFFF;

  if (!Crc16CcittTable)
  {
    crc16_ccitt_table_init();
  }

  ndx = ((crc >> 8) ^ SwapBitsInByte(data_buffer->Header)) & 0x00FF;
  crc = (*(Crc16CcittTable + ndx) ^ (crc << 8 & (~0x80000000>>7))) & ((1 << 16) - 1);

  for (unsigned i(0); i < data_buffer->Length; i++)
  {
    ndx = ((crc >> 8) ^ SwapBitsInByte(data_buffer->Data[i])) & 0x00FF;
    crc = (*(Crc16CcittTable + ndx) ^ (crc << 8 & (~0x80000000>>7))) & ((1 << 16) - 1);
  }

  return crc;
}

/************************ crcRevert ************************/
void crcRevert(uint16_t* crc)
{
  *crc = (SwapBitsInByte((*crc & 0xFF00) >> 8) << 8) | SwapBitsInByte((*crc & 0x00FF));
}

/************************ STcrc16 ************************/
void cSTSAFE_A::STcrc16(void)
{
  if (this->_Transport->ExchangeBuffer)
  {
    uint16_t crc16 = 0;

    crc16 = crc16_ccitt(this->_Transport->ExchangeBuffer);
    crc16 = SWAP2BYTES(crc16);
    crcRevert(&crc16);
    crc16 = (SWAP2BYTES(crc16)) ^ 0xFFFF;
    memcpy(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length, &crc16, sizeof(crc16));
    this->_Transport->ExchangeBuffer->Length += CRC_LENGTH;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
