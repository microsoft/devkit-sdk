/*
 * \file STSAFE_A.h
 * \brief cSTSAFE_A class definition.
 * This contains the public member function prototypes of cSTSAFE_A class.
 * \date 23/08/2016
 * \author Christophe Delaunay
 * \version 1.3
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

#ifndef STSAFE_A_H
#define STSAFE_A_H

#ifdef STSAFE_A_TYPES_H_FILE
#include STSAFE_A_TYPES_H_FILE
#else
#include "STSAFE_A_Types.h"
#endif
#include "STSAFE_A_Transport.h"
#include "STSAFE_A_Crypto.h"

/*!
 * \class cSTSAFE_A
 * \brief Representative class for cSTSAFE_A object
 * \details This class define cSTSAFE_A class object
 */
class cSTSAFE_A
{
  /********************************************************************************/
  /************************************ Public ************************************/
  /********************************************************************************/
public:
  /*!
   * \brief cSTSAFE_A class constructor.
   * \param *in_transport  : cSTSAFE_A_Transport object pointer
   * \param in_i2c_address : STSAFE-A I2C address
   * \param *in_crypto     : cSTSAFE_A_Crypto object pointer
   * \param in_hash_type   : Hash type used for asymmetric crypto session (SHA256 or SHA384)
   * \param in_crc_support : STSAFE-A CRC feature supported
   */
  cSTSAFE_A(cSTSAFE_A_Transport* in_transport, uint8_t in_i2c_address = 0x20,
            cSTSAFE_A_Crypto* in_crypto = NULL, HashTypes in_hash_type = SHA_256,
            bool in_crc_support = true);

  /*!
   * \brief cSTSAFE_A class Destructor.
   */
  ~cSTSAFE_A(void);

  /********************************************************************************/
  /* Generic STSAFE-A cmd                                                         */
  /********************************************************************************/
  /*!
   * \brief Get STSAFE-A I2C address.
   * \return I2C address value
   */
  uint8_t GetI2cAddress(void);

  /*!
   * \brief Set STSAFE-A I2C address.
   * \param in_i2c_address : I2C address value
   * \return Current STSAFE-A object I2C address value
   */
  uint8_t SetI2cAddress(uint8_t in_i2c_address);

  /*!
   * \brief Check CRC feature support.
   * \return True if CRC feature is supported by current STSAFE-A object
   */
  uint8_t GetCRCsupport(void);

  /*!
   * \brief Set CRC feature supported for current STSAFE-A object.
   * \param in_crc_support : True when CRC feature should be supported by current STSAFE-A object
   * \return True if CRC feature is supported by current STSAFE-A object
   */
  uint8_t SetCRCsupport(uint8_t in_crc_support);

  /********************************************************************************/
  /* STSAFE-A communication                                                       */
  /********************************************************************************/
  /*!
   * \brief Transmit bytes to STSAFE-A device.
   * \return OK if no error
   */
  ResponseCode Transmit(void);

  /*!
   * \brief Receive bytes from STSAFE-A device.
   * \return OK if no error
   */
  ResponseCode Receive(void);

  /********************************************************************************/
  /* General Purpose STSAFE-A command                                             */
  /********************************************************************************/
  /*!
   * \brief Echo command.
   * \param *in_echo_data   : Pointer to bytes array
   * \param in_echo_size    : Bytes array size
   * \param **out_echo      : EchoBuffer structure pointer filled with echo command response
   * \param in_mac          : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode Echo(uint8_t* in_echo_data, uint16_t in_echo_size,
                    EchoBuffer** out_echo, Mac in_mac = NO_MAC);

  /*!
   * \brief Reset command.
   * \return OK if no error
   */
  ResponseCode Reset(void);

  /*!
   * \brief Generate random command.
   * \param in_subject                : EPHEMERAL or AUTHENTICATION_CHALLENGE
   * \param in_length                 : Number of random bytes to generate
   * \param **out_generate_random     : GenerateRandomBuffer structure pointer filled with generate random command response
   * \param in_mac                    : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode GenerateRandom(RndSubject in_subject, uint8_t in_length,
                              GenerateRandomBuffer** out_generate_random,
                              Mac in_mac = NO_MAC);

  /*!
   * \brief Hibernate command.
   * \param in_wake_up_mode : Wake up mode conditions
   * \return OK if no error
   */
  ResponseCode Hibernate(WakeUpModeFromHibernate in_wake_up_mode);

  /* Start Signature Session */
  /*!
   * \brief Start signature session command.
   * \param in_slot_number                : SLOT_0 or SLOT_1 (Slot number in the private key table)
   * \param **out_start_signature_session : StartSignatureSessionBuffer structure pointer filled with start signature session command response
   * \return OK if no error
   */
  ResponseCode StartSignatureSession(KeySlotNumber in_slot_number,
                                     StartSignatureSessionBuffer** out_start_signature_session);

  /********************************************************************************/
  /* Data Partition STSAFE-A command                                              */
  /********************************************************************************/
  /*!
   * \brief Read a data partition zone command.
   * \param in_change_ac_indicator  : True if zone read access condition should be changed
   * \param in_new_read_ac_right    : True if zone read access condition right should be changed
   * \param in_new_read_ac          : New zone read access condition
   * \param in_zone_index           : Zone index
   * \param in_offset               : Offset in selected zone
   * \param in_length               : Length of data to read
   * \param **out_read              : ReadBuffer structure pointer filled with read command response
   * \param in_mac                  : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode Read(uint8_t in_change_ac_indicator, uint8_t in_new_read_ac_right,
                    AccessCondition in_new_read_ac, uint8_t in_zone_index,
                    uint16_t in_offset, uint16_t in_length,
                    ReadBuffer** out_read, Mac in_mac = NO_MAC);

  /*!
   * \brief Decrement a partition zone counter command.
   * \param in_change_ac_indicator    : True if zone decrement access condition should be changed
   * \param in_new_decrement_ac_right : True if zone decrement access condition right should be changed
   * \param in_new_decrement_ac       : New zone update access condition
   * \param in_zone_index             : Zone index
   * \param in_offset                 : Offset in selected zone
   * \param in_amount                 : Length of data to read
   * \param *in_data                  : Pointer to bytes array
   * \param **out_decrement           : DecrementBuffer structure pointer filled with decrement command response
   * \param in_mac                    : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode Decrement(uint8_t in_change_ac_indicator, uint8_t in_new_decrement_ac_right,
                         AccessCondition in_new_decrement_ac, uint8_t in_zone_index,
                         uint16_t in_offset, uint32_t in_amount, DataBuffer* in_data,
                         DecrementBuffer** out_decrement, Mac in_mac = NO_MAC);

  /*!
   * \brief Update data through zone partition without counter.
   * \param in_atomicity            : True if processing has to be atomic
   * \param in_change_ac_indicator  : True if zone update access condition should be changed
   * \param in_new_update_ac_right  : True if zone update access condition right should be changed
   * \param in_new_update_ac        : New zone update access condition
   * \param in_zone_index           : Zone index
   * \param in_offset               : Offset in selected zone
   * \param *in_data                : Pointer to bytes array
   * \param in_mac                  : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode Update(uint8_t in_atomicity, uint8_t in_change_ac_indicator,
                      uint8_t in_new_update_ac_right, AccessCondition in_new_update_ac,
                      uint8_t in_zone_index, uint16_t in_offset,
                      DataBuffer* in_data, Mac in_mac = NO_MAC);
  /********************************************************************************/
  /* Administrative STSAFE-A command                                              */
  /********************************************************************************/
  /*!
   * \brief Query product data.
   * \param **out_product_data : ProductDataBuffer structure pointer filled with product data query response
   * \param in_mac             : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode ProductDataQuery(ProductDataBuffer** out_product_data, Mac in_mac = NO_MAC);

  /*!
   * \brief Query data partition.
   * \param **out_data_partition          : DataPartitionBuffer structure pointer filled with data partition query response
   * \param **out_zone_information_record : ZoneInformationRecordBuffer structure pointer filled with zone information record
   * \param in_mac                        : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode DataPartitionQuery(DataPartitionBuffer** out_data_partition, ZoneInformationRecordBuffer** out_zone_information_record, Mac in_mac = NO_MAC);

  /*!
   * \brief Query I2C parameter.
   * \param **out_i2c_parameter : I2cParameterBuffer structure pointer filled with I2C parameter query response
   * \param in_mac              : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode I2cParameterQuery(I2cParameterBuffer** out_i2c_parameter, Mac in_mac = NO_MAC);

  /*!
   * \brief Query life cycle state.
   * \param **out_life_cycle_state : LifeCycleStateBuffer structure pointer filled with life cycle state query response
   * \param in_mac                 : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode LifeCycleStateQuery(LifeCycleStateBuffer** out_life_cycle_state, Mac in_mac = NO_MAC);

#if 0
  /*!
   * \brief Query private key mode of operation.
   * \param **out_private_key_table : PrivateKeyTableBuffer structure pointer filled with private key mode of operation query response
   * \param in_mac                : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode PrivateKeyTableQuery(PrivateKeyTableBuffer** out_private_key_table, Mac in_mac = NO_MAC);
#endif /* 0 */

  /*!
   * \brief Query public key.
   * \param **out_pub_key_slot    : PublicKeySlotBuffer structure pointer
   * \param **out_pub_x           : CoordinateBuffer structure pointer to public key X coordinate
   * \param **out_pub_y           : CoordinateBuffer structure pointer to public key Y coordinate
   * \param **out_curve_id        : CurveIdBuffer structure pointer to curve identifier
   * \param in_mac                : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode PublicKeySlotQuery(PublicKeySlotBuffer** out_pub_key_slot,
                                  CoordinateBuffer** out_pub_x, CoordinateBuffer** out_pub_y,
                                  CurveIdBuffer** out_curve_id, Mac in_mac = NO_MAC);

  /*!
   * \brief Query host key slot.
   * \param **out_host_key_slot : HostKeySlotBuffer structure pointer
   * \param in_mac              : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode HostKeySlotQuery(HostKeySlotBuffer** out_host_key_slot, Mac in_mac = NO_MAC);

  /*!
   * \brief Query local envelope key information.
   * \param **out_local_envelope_key_slot0_information_record : Local envelope key slot 0 information (sLocalEnvelopeKeyInformationRecord_t structure pointer)
   * \param **out_local_envelope_key_slot1_information_record : Local envelope key slot 1 information (sLocalEnvelopeKeyInformationRecord_t structure pointer)
   * \param in_mac                                            : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode LocalEnvelopeKeySlotQuery(LocalEnvelopeKeyTableBuffer** out_local_envelope_key_table,
                                         LocalEnvelopeKeyInformationRecordBuffer** out_local_envelope_key_slot0_information_record,
                                         LocalEnvelopeKeyInformationRecordBuffer** out_local_envelope_key_slot1_information_record,
                                         Mac in_mac = NO_MAC);

  /*!
   * \brief Put attribute to STSAFE-A.
   * \param in_attribute_tag : Attribute tag
   * \param in_data          : Value (bytes array)
   * \param in_data_size     : Value size in bytes
   * \param in_mac           : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode PutAttribute(AttributeTag in_attribute_tag, uint8_t* in_data,
                            uint16_t in_data_size, Mac in_mac = NO_MAC);

  /*!
   * \brief Generate a key-pair.
   * \param in_private_key_slot_number          : SLOT_0 or SLOT_1 (Slot number in the private key table)
   * \param in_use_limit                        : Maximum number of times the key can be used (0xFFFF : no limit)
   * \param in_change_authorization_flags_right : Changing the authorization flags is authorized when true and forbidden when false
   * \param in_authorization_flags              : Signature over command & response sequences and/or signature over a message digest and/or key establishment
   * \param in_curve_id                         : Curve identifier
   * \param **out_point_reprensentation_id      : Point representation identifier
   * \param **out_pub_x                         : CoordinateBuffer structure pointer to public key X coordinate
   * \param **out_pub_y                         : CoordinateBuffer structure pointer to public key Y coordinate
   * \param in_mac                              : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode GenerateKeyPair(KeySlotNumber in_private_key_slot_number, uint16_t in_use_limit,
                               uint8_t in_change_authorization_flags_right,
                               AuthorizationFlags in_authorization_flags,
                               CurveId in_curve_id,
                               uint8_t** out_point_reprensentation_id,
                               CoordinateBuffer** out_pub_x,
                               CoordinateBuffer** out_pub_y,
                               Mac in_mac = HOST_C_MAC);

  /*!
   * \brief Generate a local envelope key.
   * \param in_locale_envelope_key_slot_number : SLOT_0 or SLOT_1 (Slot number in the local envelope key table)
   * \param in_key_types                       : AES key types (AES128 or AES256)
   * \param *in_seed                           : Seed
   * \param in_seed_size                       : Seed size
   * \param in_mac                             : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode GenerateLocalEnveloppeKey(KeySlotNumber in_local_envelope_key_slot_number,
                                         AesTypes in_key_types, uint8_t* in_seed = NULL,
                                         uint16_t in_seed_size = 0, Mac in_mac = NO_MAC);

  /*!
   * \brief Delete password.
   * \return OK if no error
   */
  ResponseCode DeletePassword(void);

#if 0
  /*!
   * \brief Delete host key, local envelope key, public key or password.
   * \param in_key_type                       : Key type (HOST_KEY_SLOT_TAG or LOCAL_ENVELOPPE_KEY_TABLE_TAG or PUBLIC_KEY_SLOT_TAG or PASSWORD_SLOT_TAG)
   * \param in_local_envelope_key_slot_number : SLOT_0 or SLOT_1 (Slot number in the local envelope key table)
   * \param in_mac                            : MAC authenticating command
   * \return OK if no error
   */
  ResponseCode DeleteKey(AttributeTag in_key_type,
                         KeySlotNumber in_local_envelope_key_slot_number = SLOT_0,
                         Mac in_mac = NO_MAC);

  /*!
   * \brief Unwrap an issuer envelope.
   * \param *in_envelope_crypogram            : Envelope crypogram
   * \param in_envelope_crypogram_size        : Envelope crypogram size
   * \param *in_envelope_mac[]                : Envelope MAC (4 bytes)
   * \param **out_envelope_crypogram          : sEnvelopeCryptogram structure pointer to unwrap enveloppe
   * \param in_mac                            : MAC authenticating command
   * \return OK if no error
   */
  ResponseCode UnwrapIssuerEnvelope(uint8_t* in_envelope_crypogram,
                                    uint16_t in_envelope_crypogram_size,
                                    uint8_t in_envelope_mac[4],
                                    sEnvelopeCryptogram** out_envelope_crypogram,
                                    Mac in_mac = HOST_C_MAC);
#endif /* 0 */

  /*!
   * \brief Wrap a local envelope.
   * \param in_locale_envelope_key_slot_number : SLOT_0 or SLOT_1 (Slot number in the local envelope key table)
   * \param *in_data                           : Data
   * \param in_data_size                       : Data size (non-zero multiple of 8 bytes and maximum 480 bytes)
   * \param **out_local_envelope               : LocalEnvelopeBuffer structure pointer to wrapped local envelope
   * \param in_mac                             : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode WrapLocalEnvelope(KeySlotNumber in_locale_envelope_key_slot_number,
                                 uint8_t* in_data, uint16_t in_data_size,
                                 LocalEnvelopeBuffer** out_local_envelope,
                                 Mac in_mac = HOST_C_MAC);

  /*!
   * \brief Unwrap a local envelope.
   * \param in_local_envelope_key_slot_number : SLOT_0 or SLOT_1 (Slot number in the local envelope key table)
   * \param *in_local_envelope                : Local envelope to unwrap
   * \param in_local_envelope_size            : Local envelope size
   * \param **out_data                        : DataBuffer structure pointer to working key
   * \param in_mac                            : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode UnwrapLocalEnvelope(KeySlotNumber in_local_envelope_key_slot_number,
                                   uint8_t* in_local_envelope, uint16_t in_local_envelope_size,
                                   DataBuffer** out_data, Mac in_mac = HOST_C_MAC);

  /*!
   * \brief Verify password.
   * \param *in_password          : Password (16 bytes length)
   * \param **out_verify_password : VerifyPasswordBuffer structure pointer to verify password command response
   * \param in_mac                : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode VerifyPassword(uint8_t* in_password, VerifyPasswordBuffer** out_verify_password,
                              Mac in_mac = NO_MAC);

  /********************************************************************************/
  /* private & public STSAFE-A key command                                        */
  /********************************************************************************/
  /*!
   * \brief Get signature over commands & responses sequence.
   * \param *in_challenge         : Pointer to challenge bytes array
   * \param in_challenge_size     : Challenge size in term of bytes
   * \param **out_signature       : SignatureBuffer structure pointer to signature
   * \param in_mac                : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode GetSignature(uint8_t* in_challenge, uint16_t in_challenge_size,
                            SignatureBuffer** out_signature, Mac in_mac = NO_MAC);

  /*!
   * \brief Generate a signature over a message digest.
   * \param in_private_key_slot_number : SLOT_0 or SLOT_1 (Slot number in the private key table)
   * \param *in_digest                 : Pointer to message digest
   * \param *in_digest_type            : Message digest size (32 or 48 bytes)
   * \param **out_signature            : SignatureBuffer structure pointer to signature
   * \param in_mac                     : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode GenerateSignature(KeySlotNumber in_private_key_slot_number,
                                 uint8_t* in_digest, HashTypes in_digest_type,
                                 SignatureBuffer** out_signature, Mac in_mac = NO_MAC);

  /*!
   * \brief Verify message's signature.
   * \param in_curve_id             : Curve identifier
   * \param *in_pub_x               : CoordinateBuffer structure defining public key X coordinate
   * \param *in_pub_y               : CoordinateBuffer structure defining public key Y coordinate
   * \param *in_r_signature         : SignatureBuffer structure defining R of ECDSA signature
   * \param *in_s_Signature         : SignatureBuffer structure defining S of ECDSA signature
   * \param *in_digest              : DataBuffer structure defining the message digest
   * \param **out_verify_signature  : VerifySignatureBuffer structure pointer filled with verify signature command response
   * \param in_mac                  : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode VerifyMessageSignature(CurveId in_curve_id, CoordinateBuffer* in_pub_x, CoordinateBuffer* in_pub_y,
                                      SignatureBuffer* in_r_signature, SignatureBuffer* in_s_Signature,
                                      DataBuffer* in_digest, VerifySignatureBuffer** out_verify_signature,
                                      Mac in_mac = NO_MAC);

  /*!
   * \brief Verify entity's signature.
   * \param *in_r_signature         : SignatureBuffer structure defining R of ECDSA signature
   * \param *in_s_signature         : SignatureBuffer structure defining S of ECDSA signature
   * \param **out_verify_signature  : VerifySignatureBuffer structure pointer filled with verify signature command response
   * \param in_mac                  : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode VerifyEntitySignature(SignatureBuffer* in_r_signature, SignatureBuffer* in_s_signature,
                                     VerifySignatureBuffer** out_verify_signature,
                                     Mac in_mac = NO_MAC);

  /*!
   * \brief Establish Key command.
   * \param in_priv_key_slot_number : SLOT_0 or SLOT_1 (Slot number in the private key table)
   * \param *in_pub_x               : CoordinateBuffer structure defining public key X coordinate
   * \param *in_pub_y               : CoordinateBuffer structure defining public key Y coordinate
   * \param **out_shared_secret     : SharedSecretBuffer structure pointer filled with get signature command response
   * \param in_mac                  : MAC authenticating command/response
   * \return OK if no error
   */
  ResponseCode EstablishKey(KeySlotNumber in_priv_key_slot_number,
                            CoordinateBuffer* in_pub_x,
                            CoordinateBuffer* in_pub_y,
                            SharedSecretBuffer** out_shared_secret,
                            Mac in_mac = HOST_C_MAC);

  /********************************************************************************/
  /* Data buffer command                                                          */
  /********************************************************************************/
  /*!
   * \brief Configure automatically data exchange buffer with CRC support or not.
   * \return OK if no error
   */
  ResponseCode DataBufferConfiguration(void);

  /*!
   * \brief Get a pointer to data buffer.
   * \return Pointer to input/output buffer
   */
  DataExchangeBuffer* GetDataBuffer(void);

  /*!
   * \brief Get data buffer size.
   * \return Input/output buffer size
   */
  uint16_t GetDataBufferSize(void);

  /********************************************************************************/
  /* Tools command                                                                */
  /********************************************************************************/
  /*!
   * \brief Extract certificate.
   * \param **out_certificate     : Pointer to certificate array to be filled
   * \param *out_certificate_size : Pointer to certificate array size
   * \param in_zone_index         : Index of zone where read the certificate
   * \return OK if no error
   */
  ResponseCode ExtractCertificate(uint8_t** out_certificate, uint16_t* out_certificate_size,
                                  uint8_t in_zone_index = 0);

  /*!
   * \brief Set host cipher key.
   * \param in_host_cipher_key[]  : 128bits Host cipher key
   */
  void SetHostCipherKey(uint8_t in_host_cipher_key[HOST_KEY_LENGTH/8]);

  /*!
   * \brief Set host MAC key.
   * \param in_host_mac_key[]     : 128bits Host MAC key
   */
  void SetHostMacKey(uint8_t in_host_mac_key[HOST_KEY_LENGTH/8]);

  /*!
   * \brief Compute initial value.
   * \param in_subject            : Subject (CMAC_COMPUTATION or RMAC_COMPUTATION or C_ENCRYPTION or R_ENCRYPTION)
   * \param out_initial_value[]   : Initial value output (16 bytes)
   */
  void ComputeInitialValue(InitialValue in_subject,
                           uint8_t out_initial_value[HOST_KEY_LENGTH/8]);

  /*!
   * \brief AES CBC data encryption.
   * \return OK if no error
   */
  ResponseCode DataEncryption(void);

  /*!
   * \brief AES CBC data decryption.
   * \return OK if no error
   */
  ResponseCode DataDecryption(void);

  /*!
   * \brief Compute CMAC value.
   */
  ResponseCode ComputeCMAC(void);

  /*!
   * \brief Compute RMAC value.
   */
  void PrecomputeRMAC(void);

  /*!
   * \brief Compute RMAC value.
   */
  void ComputeRMAC(void);

  /*!
   * \brief Set MAC counter.
   * \param in_mac_counter : MAC counter value (7 bits)
   */
  void SetMACCounter(uint8_t in_mac_counter);

  /*!
   * \brief Get MAC counter.
   * \return MAC counter value (7 bits)
   */
  uint8_t GetMACCounter(void);

  /*!
   * \brief Get Host CMAC sequence counter.
   * \return Host C-MAC sequence counter value (3 bytes)
   */
  uint32_t GetHostCMacSequenceCounter(void);

  /********************************************************************************/
  /* Hash STSAFE-A command                                                        */
  /********************************************************************************/
  /* Get Hash */
  /*!
   * \brief Get Hash array pointer.
   * \return Hash array pointer
   */
  uint8_t* GetHashBuffer(void);

  /*!
   * \brief Set Hash type.
   * \param in_hash_type : Hash type
   * \return Hash type used (SHA-256 or SHA-384)
   */
  HashTypes SetHashType(HashTypes in_hash_type);

  /*!
   * \brief Get Hash type.
   * \return Hash type used (SHA-256 or SHA-384)
   */
  HashTypes GetHashType(void);

  /********************************************************************************/
  /* ********************************* Private ************************************/
  /********************************************************************************/
private:
  /* I2C address */
  uint8_t _i2cAddress;

  /* CRC support */
  uint8_t _CRCsupported;

  /* I2c transport */
  cSTSAFE_A_Transport* _Transport;

  /* Calculate CRC */
  void STcrc16(void);

  /*Crypto ctx */
  cSTSAFE_A_Crypto* _CryptoCtx;

  /*Crypto hash */
  HashTypes _HashType;
  uint8_t _hash[HASH_SIZE(SHA_384)];

  /*Crypto counters */
  uint8_t _HostCMacSequenceCounter[3]; /* Host-MAC counter */
  uint8_t _MacCounter;                 /* MAC counter for admin/peripheral MACs */

  /*Crypto keys */
  uint8_t _HostCipherKey[HOST_KEY_LENGTH/8]; /* Host cipher key */
  uint8_t _HostMacKey[HOST_KEY_LENGTH/8];    /* Host-Mac key */
};

#endif /* STSAFE_A_H */
