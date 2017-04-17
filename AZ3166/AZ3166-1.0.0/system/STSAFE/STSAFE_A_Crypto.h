/*
 * \file STSAFE_A_Crypto.h
 * \brief cSTSAFE_A_Crypto class definition.
 * This contains the public member function prototypes of cSTSAFE_A_Crypto class.
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

#ifndef STSAFE_A_CRYPTO_H
#define STSAFE_A_CRYPTO_H

#ifdef STSAFE_A_TYPES_H_FILE
#include STSAFE_A_TYPES_H_FILE
#else
#include "STSAFE_A_Types.h"
#endif /* STSAFE_A_TYPES_H_FILE */

/*!
 * \class cSTSAFE_A_Crypto
 * \brief Representative class for cSTSAFE_A_Crypto object
 * \details This class define cSTSAFE_A_Crypto object
 */
class cSTSAFE_A_Crypto
{
public:
  /* Hash */
  virtual int32_t Sha_Init(HashTypes hash_type) = 0;
  virtual int32_t Sha_Update(HashTypes hash_type, uint8_t* input_message,
                             uint32_t input_message_length) = 0;
  virtual int32_t Sha_Final(HashTypes hash_type, uint8_t* message_digest,
                            uint32_t* message_digest_length = NULL) = 0;
  virtual bool Check_Sha_Context(HashTypes hash_type) = 0;
  /* AES */
  virtual int32_t AES_ecb_encrypt(uint8_t* in, uint8_t* out, uint8_t* key, AesTypes aes_type) = 0;
  virtual int32_t AES_cbc_encrypt(uint8_t* in, uint16_t length, uint8_t* out,
                                  uint8_t* key, uint8_t* iv, AesTypes aes_type) = 0;
  virtual int32_t AES_cbc_decrypt(uint8_t* in, uint16_t length, uint8_t* out,
                                  uint8_t* key, uint8_t* iv, AesTypes aes_type) = 0;

  virtual void AES_mac_start(uint8_t* key, Mac mac_type = C_MAC) = 0;
  virtual void AES_mac_update(uint8_t* data, uint16_t length, Mac mac_type = C_MAC) = 0;
  virtual void AES_mac_final(uint8_t* cmac, Mac mac_type = C_MAC) = 0;
};

#endif /* STSAFE_A_CRYPTO_H */
