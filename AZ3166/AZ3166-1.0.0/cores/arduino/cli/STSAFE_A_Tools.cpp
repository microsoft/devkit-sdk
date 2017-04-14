/*
 * \file STSAFE_A_Tools.cpp
 * \brief STSAFE-A Tools routine.
 * \date 23/08/2016
 * \author Christophe Delaunay
 * \version 1.4
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

/* MIN */
#define MIN(a,b) (((a)<(b))?(a):(b))


/*****************************/
/* Data buffer configuration */
/*****************************/
/*
 * This command is executed to configure the input/output buffer size as
 * defined through STSAFE-A device and set CRC flag following CRC feature supported or not.
 */
ResponseCode cSTSAFE_A::DataBufferConfiguration(void)
{
  ResponseCode status_code = UNEXPECTED_ERROR;

  if (!this->_Transport->ExchangeBuffer)
  {
    this->_Transport->ExchangeBuffer = (DataExchangeBuffer*)STS_NEW(uint8_t, this->_Transport->ExchangeBufferSize + 4);
  }

  /* Check if CRC is supported */
  if (this->_Transport->ExchangeBuffer && this->SetCRCsupport(1))
  {
    ProductDataBuffer* sts_product_data;
    status_code = this->ProductDataQuery(&sts_product_data);

    if (status_code)
    {
      this->SetCRCsupport(0);
      status_code = this->ProductDataQuery(&sts_product_data);
    }

    if (!status_code)
    {
      /* Reallocate I2C buffer to defined size through STSAFE-A device */
      if (this->_Transport->ExchangeBufferSize > MIN(sts_product_data->InputOutputBufferSize * sizeof(uint8_t), this->_Transport->ExchangeBufferSize))
      {
        this->_Transport->ExchangeBufferSize = MIN(sts_product_data->InputOutputBufferSize * sizeof(uint8_t), this->_Transport->ExchangeBufferSize);
        STS_DELETE(this->_Transport->ExchangeBuffer);
        this->_Transport->ExchangeBuffer = (DataExchangeBuffer*)STS_NEW(uint8_t, this->_Transport->ExchangeBufferSize + 4);
        if (!this->_Transport->ExchangeBuffer)
          return (UNEXPECTED_ERROR);
        }
    }
  }

  return status_code;
}

/************************/
/* Extract certif       */
/************************/
/*
 * This command is executed to get certificate stored through a specified zone.
 */
ResponseCode cSTSAFE_A::ExtractCertificate(uint8_t** out_certificate,
	                                   uint16_t* out_certificate_size,
	                                   uint8_t in_zone_index)
{
  ReadBuffer* sts_read = NULL;
  uint8_t status_code = this->Read(0, 0, ALWAYS, in_zone_index, 0, 4, &sts_read);

  *out_certificate_size = 0;

  if (!status_code && (sts_read->Length == 0x04) && (sts_read->Data[0] == 0x30))
  {
    switch (sts_read->Data[1])
    {
      case 0x81:
        {
          *out_certificate_size = sts_read->Data[2] + 3;
        }
        break;
      case 0x82:
        {
          *out_certificate_size = (sts_read->Data[2] << 8) + sts_read->Data[3] + 4;
        }
        break;
      default:
        {
          *out_certificate_size = sts_read->Data[1];
          if (*out_certificate_size < 0x81)
          {
            *out_certificate_size += 2;
          }
          else
          {
            *out_certificate_size = 0;
          }
        }
        break;
    }
  }

  if (!status_code && *out_certificate_size)
  {
    *out_certificate = (uint8_t*)STS_NEW(uint8_t, *out_certificate_size);

    if (*out_certificate)
    {
			uint16_t i;
      uint8_t step = 256 - (sizeof(DataExchangeBuffer) / sizeof(uint8_t) - 1) - (this->_CRCsupported ? 2 : 0);
      for (i = 0; i < *out_certificate_size / step; i++)
      {
        status_code = this->Read(0, 0, ALWAYS, in_zone_index, i * step, step, &sts_read);
        memcpy(*out_certificate + (i * step), sts_read->Data, sts_read->Length);
      }
      if ((!status_code) && (*out_certificate_size % step))
      {
        status_code |= this->Read(0, 0, ALWAYS, in_zone_index, i * step, *out_certificate_size % step, &sts_read);
        memcpy(*out_certificate + (i * step), sts_read->Data, sts_read->Length);
      }
    }
  }

  return (ResponseCode)status_code;
}

/*********************** */
/* Set Hash Type */
/*********************** */
/*
 * This command is executed to set right hash function (SHA-256 or SHA-384).
 */
HashTypes cSTSAFE_A::SetHashType(HashTypes in_hash_type)
{
  this->_HashType = in_hash_type;
  return (this->GetHashType());
  this->_HashType = in_hash_type;
}

/************************/
/* Get Hash Type        */
/************************/
/*
 * This command is executed to set right hash function (SHA-256 or SHA-384).
 */
HashTypes cSTSAFE_A::GetHashType(void)
{
  return this->_HashType;
}

#ifndef _AUTHENT_ONLY_
/************************/
/* Get Data buffer      */
/************************/
/*
 * This command is executed to get data buffer pointer to be able
 * to set or get bytes through buffer.
 */
DataExchangeBuffer* cSTSAFE_A::GetDataBuffer(void)
{
  return this->_Transport->ExchangeBuffer;
}

/************************/
/* Get Data buffer size */
/************************/
/*
 * This command is executed to get input/output buffer size.
 */
uint16_t cSTSAFE_A::GetDataBufferSize(void)
{
  return this->_Transport->ExchangeBufferSize;
}

/************************/
/* Get Hash             */
/************************/
/*
 * This command is executed to get hash buffer pointer where hash value is stored.
 */
uint8_t* cSTSAFE_A::GetHashBuffer(void)
{
  return this->_hash;
}

/************************/
/* Set Host Cipher Key  */
/************************/
/*
 * This command is executed to store the host cipher key.
 */
void cSTSAFE_A::SetHostCipherKey(uint8_t in_host_cipher_key[HOST_KEY_LENGTH / 8])
{
  memcpy(this->_HostCipherKey, in_host_cipher_key, (HOST_KEY_LENGTH / 8) * sizeof(in_host_cipher_key[0]));
}

/************************/
/* Set Host MAC Key     */
/************************/
/*
 * This command is executed to store the host MAC key.
 */
void cSTSAFE_A::SetHostMacKey(uint8_t in_host_mac_key[HOST_KEY_LENGTH / 8])
{
  memcpy(this->_HostMacKey, in_host_mac_key, (HOST_KEY_LENGTH / 8) * sizeof(in_host_mac_key[0]));
}

/*************************/
/* Compute initial value */
/*************************/
/*
 * This command is executed to compute the initial value used for CMAC/RMAC computation,
 * C-Encryption or R-Encryption.
 */
void cSTSAFE_A::ComputeInitialValue(InitialValue in_subject,
                                    uint8_t out_initial_value[HOST_KEY_LENGTH / 8])
{
  uint32_t host_cmac_sequence_counter =
        ((this->_HostCMacSequenceCounter[0] << 16) |
         (this->_HostCMacSequenceCounter[1] << 8) |
          this->_HostCMacSequenceCounter[2]);

  if (in_subject != CMAC_COMPUTATION)
  {
    host_cmac_sequence_counter++;
  }

  out_initial_value[0] = (host_cmac_sequence_counter >> 16) & 0xFF;
  out_initial_value[1] = (host_cmac_sequence_counter >> 8) & 0xFF;
  out_initial_value[2] = host_cmac_sequence_counter & 0xFF;
  out_initial_value[3] = (uint8_t)((in_subject << 6) & 0xC0);
  out_initial_value[4] = 0x80;
  memset(out_initial_value + 5, 0x00, 11 * sizeof(out_initial_value[0]));
}

/************************/
/* Data encryption      */
/************************/
/*
 * This command is executed to encrypt data.
 */
ResponseCode cSTSAFE_A::DataEncryption(void)
{
  ResponseCode status_code = CRYPTO_LIB_ISSUE;

  if (this->_CryptoCtx)
  {
    /* Compute initial value */
    uint8_t initial_value[HOST_KEY_LENGTH / 8];
    this->ComputeInitialValue(C_ENCRYPTION, initial_value);
    status_code = (this->_CryptoCtx->AES_ecb_encrypt(initial_value, initial_value, this->_HostCipherKey, AES_128) ? CRYPTO_LIB_ISSUE : OK);

    /* Encrypt data */
    if (!status_code)
    {
      this->_Transport->ExchangeBuffer->Data[this->_Transport->ExchangeBuffer->Length] = 0x80;
      this->_Transport->ExchangeBuffer->Length++;
      uint8_t padding_length = (16 - (this->_Transport->ExchangeBuffer->Length % 16) % 16);
      memset(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length, 0x00, padding_length * sizeof(padding_length));
      this->_Transport->ExchangeBuffer->Length += padding_length;
      status_code = this->_CryptoCtx->AES_cbc_encrypt(this->_Transport->ExchangeBuffer->Data,
                                                      this->_Transport->ExchangeBuffer->Length,
                                                      this->_Transport->ExchangeBuffer->Data,
                                                      this->_HostCipherKey, initial_value,
                                                      AES_128) ? CRYPTO_LIB_ISSUE : OK;
    }
  }

  return status_code;
}

/************************/
/* Data decryption      */
/************************/
/*
 * Reverse memchr() : Find the last occurrence of 'c' in the buffer 's' of size 'n'.
 */
static inline void *gitmemrchr(const void *s, int32_t c, size_t n)
{
  const uint8_t* p = (uint8_t*)s;
  p += n;
  while (p != s)
  {
    if (*--p == (uint8_t) c)
    {
      return (void *)p;
    }
  }

  return NULL;
}

/*
 * This command is executed to decrypt data.
 */
ResponseCode cSTSAFE_A::DataDecryption(void)
{
  uint8_t status_code = CRYPTO_LIB_ISSUE;
  if (this->_CryptoCtx)
  {
    /* Compute initial value */
    uint8_t initial_value[HOST_KEY_LENGTH / 8];
    this->ComputeInitialValue(R_ENCRYPTION, initial_value);
    status_code = (this->_CryptoCtx->AES_ecb_encrypt(initial_value, initial_value, this->_HostCipherKey, AES_128) ? CRYPTO_LIB_ISSUE : OK);

    /* Decrypt data */
    if (!status_code)
    {
      status_code = this->_CryptoCtx->AES_cbc_decrypt(this->_Transport->ExchangeBuffer->Data,
                                                      this->_Transport->ExchangeBuffer->Length,
                                                      this->_Transport->ExchangeBuffer->Data,
                                                      this->_HostCipherKey, initial_value,
                                                      AES_128) ? CRYPTO_LIB_ISSUE : OK;
      if (!status_code)
      {
#if 0
        uint16_t length = this->_Transport->ExchangeBuffer->Length;
        while ( (*(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length - 1) == 0x00) && (this->_Transport->ExchangeBuffer->Length > (length - 15)) )
          this->_Transport->ExchangeBuffer->Length --;
        if (*(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length - 1) == 0x80)
          this->_Transport->ExchangeBuffer->Length --;
        else
          status_code = CRYPTO_LIB_ISSUE;
#endif /* 0 */

        uint8_t* padding = (uint8_t*)gitmemrchr(this->_Transport->ExchangeBuffer->Data, 0x80, this->_Transport->ExchangeBuffer->Length);
        if ((!padding) ||
            (padding > (this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length - 1)) ||
            ((padding + 1 - this->_Transport->ExchangeBuffer->Data) < (this->_Transport->ExchangeBuffer->Length - 15)))
        {
          status_code = CRYPTO_LIB_ISSUE;
        }
        else
        {
          this->_Transport->ExchangeBuffer->Length = padding - this->_Transport->ExchangeBuffer->Data;
        }
      }
    }
  }

  return (ResponseCode) status_code;
}

/************************/
/* Compute CMAC         */
/************************/
/*
 * This command is executed for CMAC computation.
 */
ResponseCode cSTSAFE_A::ComputeCMAC(void)
{
  if (this->_HostMacKey && this->_CryptoCtx)
  {
    uint8_t* cmac_computation_input = (uint8_t*)STS_NEW(uint8_t, this->_Transport->ExchangeBuffer->Length + 20);
    uint16_t k = 0;
    uint8_t host_cmac_computation = this->_Transport->ExchangeBuffer->Header & H_MAC;
    if (host_cmac_computation)
    {
      this->ComputeInitialValue(CMAC_COMPUTATION, cmac_computation_input);
      k = 16;
    }
    cmac_computation_input[k] = host_cmac_computation ? 0x00 : this->_MacCounter & 0x7F;
    cmac_computation_input[k + 1] = this->_Transport->ExchangeBuffer->Header;
    uint16_t length = SWAP2BYTES(this->_Transport->ExchangeBuffer->Length);
    memcpy(cmac_computation_input + k + 2, &length, sizeof(length));
    memcpy(cmac_computation_input + k + 4,
           &this->_Transport->ExchangeBuffer->Data,
           this->_Transport->ExchangeBuffer->Length * sizeof(this->_Transport->ExchangeBuffer->Data[0]));
    this->_CryptoCtx->AES_mac_start(this->_HostMacKey);
    this->_CryptoCtx->AES_mac_update(cmac_computation_input, this->_Transport->ExchangeBuffer->Length + (host_cmac_computation ? 20 : 4));
    this->_CryptoCtx->AES_mac_final(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length);
    this->_Transport->ExchangeBuffer->Length += 4;
    STS_DELETE(cmac_computation_input);
    return OK;
  }
  else
    return CRYPTO_LIB_ISSUE;
}

/************************/
/* Compute RMAC         */
/************************/
/*
 * This command is executed for RMAC computation over command.
 */
void cSTSAFE_A::PrecomputeRMAC(void)
{
  if (this->_HostMacKey)
  {

    uint8_t* rmac_computation_input = (uint8_t*)STS_NEW(uint8_t, this->_Transport->ExchangeBuffer->Length + 20);
    uint16_t k = 0;
    uint8_t host_rmac_computation = this->_Transport->ExchangeBuffer->Header & H_MAC;
    if (host_rmac_computation)
    {
      this->ComputeInitialValue(RMAC_COMPUTATION, rmac_computation_input);
      k = 16;
    }
    rmac_computation_input[k] = host_rmac_computation ? 0x80 : this->_MacCounter | 0x80;
    rmac_computation_input[k + 1] = this->_Transport->ExchangeBuffer->Header;
    uint16_t length = SWAP2BYTES(this->_Transport->ExchangeBuffer->Length);
    memcpy(rmac_computation_input + k + 2, &length, sizeof(length));
    memcpy(rmac_computation_input + k + 4, &this->_Transport->ExchangeBuffer->Data, this->_Transport->ExchangeBuffer->Length * sizeof(this->_Transport->ExchangeBuffer->Data[0]));
    this->_CryptoCtx->AES_mac_start(this->_HostMacKey, R_MAC);
    this->_CryptoCtx->AES_mac_update(rmac_computation_input, this->_Transport->ExchangeBuffer->Length + (host_rmac_computation ? 20 : 4), R_MAC);
    STS_DELETE(rmac_computation_input);
  }
}

/*
 * This command is executed for RMAC computation over response.
 */
void cSTSAFE_A::ComputeRMAC(void)
{
  if (this->_HostMacKey)
  {
    uint8_t* rmac_computation_input = (uint8_t*)STS_NEW(uint8_t, this->_Transport->ExchangeBuffer->Length + 3);
    rmac_computation_input[0] = this->_Transport->ExchangeBuffer->Header;
    uint16_t length = SWAP2BYTES(this->_Transport->ExchangeBuffer->Length);
    memcpy(rmac_computation_input + 1, &length, sizeof(length));
    memcpy(rmac_computation_input + 3, &this->_Transport->ExchangeBuffer->Data, this->_Transport->ExchangeBuffer->Length * sizeof(this->_Transport->ExchangeBuffer->Data[0]));
    this->_CryptoCtx->AES_mac_update(rmac_computation_input, this->_Transport->ExchangeBuffer->Length + 3, R_MAC);
    this->_CryptoCtx->AES_mac_final(this->_Transport->ExchangeBuffer->Data + this->_Transport->ExchangeBuffer->Length, R_MAC);
    this->_Transport->ExchangeBuffer->Length += 4;
    STS_DELETE(rmac_computation_input);
  }
}

/************************/
/* Set MAC counter      */
/************************/
/*
 * This command is executed to set MAC counter. MAC counter is used
 * for Admin/peripheral/service MACs
 */
void cSTSAFE_A::SetMACCounter(uint8_t in_mac_counter)
{
  this->_MacCounter = in_mac_counter;
}

/************************/
/* Get MAC counter      */
/************************/
/*
 * This command is executed to Get MAC counter. MAC counter is used
 * for Admin/peripheral/service MACs
 */
uint8_t cSTSAFE_A::GetMACCounter(void)
{
  return this->_MacCounter;
}

/***********************************/
/* Get host C-MAC sequence counter */
/***********************************/
/*
 * This command is executed to Get MAC counter. MAC counter is used
 * for Admin/peripheral/service MACs
 */
uint32_t cSTSAFE_A::GetHostCMacSequenceCounter(void)
{
  HostKeySlotBuffer* host_key_slot;
  if (!HostKeySlotQuery(&host_key_slot))
    memcpy(this->_HostCMacSequenceCounter,
           host_key_slot->HostCMacSequenceCounter,
           sizeof(this->_HostCMacSequenceCounter) * sizeof(this->_HostCMacSequenceCounter[0]));

  return (((this->_HostCMacSequenceCounter[0] << 16) |
           (this->_HostCMacSequenceCounter[1] << 8) |
            this->_HostCMacSequenceCounter[2]));
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
