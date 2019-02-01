/*(Copyright)

Microsoft Copyright 2015, 2016
Confidential Information

*/

#include "RiotCore.h"
#include <stdio.h>
#include "azure_c_shared_utility/xlogging.h"

#define logging 0

// RIoT Core data
static uint8_t cDigest[RIOT_DIGEST_LENGTH+1];
static uint8_t FWID[RIOT_DIGEST_LENGTH+1];
static uint8_t derBuffer[DER_MAX_TBS];

// Keys and Certs for upper layer(s)
char g_DeviceIDPub[DER_MAX_PEM] = {0};
char g_AliasKey[DER_MAX_PEM] = {0};
char g_AKCert[DER_MAX_PEM] = {0};
char g_DICert[DER_MAX_PEM] = {0};

// Lengths
unsigned int g_DeviceIDPubLen;
unsigned int g_AliasKeyLen;
unsigned int g_AKCertLen;
unsigned int g_DICertLen;

// We don't do CSRs on this device
static bool SelfSignedDeviceCert = true;

// attribute section info from AZ3166.ld
extern void* __start_riot_fw;
extern void* __stop_riot_fw;

static bool riot_enabled = false;

bool IsIndividualRIoTIsEnabled()
{
    return riot_enabled;
}

char * RIoTGetDeviceID(unsigned int *len)
{
    if (len)
        *len = g_DeviceIDPubLen;
    return g_DeviceIDPub;
}

char * RIoTGetAliasKey(unsigned int *len)
{ 
    if (len)
        *len = g_AliasKeyLen;
    return g_AliasKey;
}

char * RIoTGetAliasCert(unsigned int *len)
{
    if (len)
        *len = g_AKCertLen;
    return g_AKCert;
}

char * RIoTGetDeviceCert(unsigned int *len)
{
    if (len)
        *len = g_DICertLen;
    return g_DICert;
}

// The static data fields that make up the Alias Cert "to be signed" region
RIOT_X509_TBS_DATA x509AliasTBSData = { { 0x0A, 0x0B, 0x0C, 0x0D, 0x0E },
                                       "devkitdice", "DEVKIT_TEST", "US",
                                       "170101000000Z", "370101000000Z",
                                       NULL, "DEVKIT_TEST", "US" };

// The static data fields that make up the DeviceID Cert "to be signed" region
RIOT_X509_TBS_DATA x509DeviceTBSData = { { 0x0E, 0x0D, 0x0C, 0x0B, 0x0A },
                                       "devkitdice", "DEVKIT_TEST", "US",
                                       "170101000000Z", "370101000000Z",
                                       "devkitdice", "DEVKIT_TEST", "US" };

extern int GetMACWithoutColon(char* buff);
extern const char* GetBoardID(void);
extern const char* getDevkitVersion();

// Entry point for RIoT Invariant Code
int RiotStart(uint8_t *CDI, uint16_t CDILen, const char *RegistrationId)
{
    int status;
    
    RIOT_ECC_PUBLIC     deviceIDPub, aliasKeyPub;
    RIOT_ECC_PRIVATE    deviceIDPriv, aliasKeyPriv;
    RIOT_ECC_SIGNATURE  tbsSig;
    DERBuilderContext   derCtx;
    uint8_t             *base;
    uint32_t            length;
    uint32_t            dcType;

    riot_enabled = false;

    // Verify registrationId input and given an empty input, generate one based on MAC address and firmware version of your DevKit
    int regIdLength = strlen(RegistrationId);
    char * realRegistrationId;

    if (RegistrationId == NULL || regIdLength == 0){
        char macAddress[24] = { "\0" };
        GetMACWithoutColon(macAddress);
        LogInfo("DevKit MAC Address: %s", macAddress);

        const char *boardId = GetBoardID();
        const char *fmVersion = getDevkitVersion();
        LogInfo("DevKit Firmware Version: %s", fmVersion);

        char generatedRegistrationId[AUTO_GEN_REGISTRATION_ID_MAX_LENGTH] = { "\0" };
        snprintf(generatedRegistrationId, AUTO_GEN_REGISTRATION_ID_MAX_LENGTH, "%sv%s", boardId, fmVersion);
        
        for(int i = 0; i < (int)strlen(generatedRegistrationId); i++){
            if(generatedRegistrationId[i] == '.'){
                generatedRegistrationId[i] = 'v';
            }
        }

        // Update subject common of alias certificate TBD data to auto-generated registration Id
        realRegistrationId = strdup(generatedRegistrationId);
    }
    else if (regIdLength > REGISTRATION_ID_MAX_LENGTH){
        LogError("Input value for registrationId in DPS.ino exceeds maximum length %d.", REGISTRATION_ID_MAX_LENGTH);
        status = RIOT_INVALID_DEVICE_ID;
        return status;
    }
    else{
        for (int i = 0; i < regIdLength; i++){
            if ((RegistrationId[i] >= 'a' && RegistrationId[i] <= 'z') 
            || (RegistrationId[i] >= '0' && RegistrationId[i] <= '9') 
            || (RegistrationId[i] == '-')){
                continue;
            }
            else{
                LogError("Input value for registrationId in DPS.ino does not meet DPS requirements - only alphanumeric, lowercase, and hyphen are supported.");
                status = RIOT_INVALID_DEVICE_ID;
                return status;
            }
        }
        // Update subject common of alias certificate TBD data to input registration Id
        realRegistrationId = strdup(RegistrationId);
    }

    x509AliasTBSData.SubjectCommon = realRegistrationId;

    // Validate Compound Device Identifier, pointer and length
    if (!(CDI) || (CDILen != RIOT_DIGEST_LENGTH)) {
        // Invalid state, remediate.
        LogError("Failure: Invalid CDI information from DICE.");
        return RIOT_INVALID_PARAMETER;
    }

    // Don't use CDI directly
    if ((status = RiotCrypt_Hash(cDigest, RIOT_DIGEST_LENGTH, CDI, CDILen)) != RIOT_SUCCESS){
        LogError("Failure: RiotCrypt_Hash returned invalid status %d.", status);
        return status;
    }

    // Derive DeviceID key pair from CDI
    if ((status = RiotCrypt_DeriveEccKey(&deviceIDPub, &deviceIDPriv, cDigest, RIOT_DIGEST_LENGTH,
        (const uint8_t *)RIOT_LABEL_IDENTITY, lblSize(RIOT_LABEL_IDENTITY))) != RIOT_SUCCESS){
        LogError("Failure: RiotCrypt_DeriveEccKey returned invalid status %d.", status);
        return status;
    }

    // Pickup start of FW image and calculate its length
    // It must be the address of DPS code
    base = (uint8_t*)&__start_riot_fw;
    length = (uint8_t*)&__stop_riot_fw - base;

#if logging
    LogInfo("The riot_fw start address: %p", &__start_riot_fw);
    LogInfo("The riot_fw end address: %p", &__stop_riot_fw);

    LogInfo("Riot FW code:");
    for(int i = 0; i < length; i++){
        if(i == (length - 1)){
            printf("%02x\r\n", base[i]);
        }
        else{
            printf("%02x", base[i]);
        }
    }
#endif

    // Measure FW, i.e., calculate FWID
    if ((status = RiotCrypt_Hash(FWID, RIOT_DIGEST_LENGTH, base, length)) != RIOT_SUCCESS){
        LogError("Failure: RiotCrypt_Hash returned invalid status %d.", status);
        return status;
    }
    FWID[RIOT_DIGEST_LENGTH] = '\0';

    // Combine CDI and FWID, result in cDigest
    if ((status = RiotCrypt_Hash2(cDigest, RIOT_DIGEST_LENGTH, cDigest, RIOT_DIGEST_LENGTH, 
    FWID, RIOT_DIGEST_LENGTH)) != RIOT_SUCCESS){
        LogError("Failure: RiotCrypt_Hash2 returned invalid status %d.", status);
        return status;
    }
    cDigest[RIOT_DIGEST_LENGTH] = '\0';

    // Derive Alias key pair from CDI and FWID
    if ((status = RiotCrypt_DeriveEccKey(&aliasKeyPub, &aliasKeyPriv, cDigest, RIOT_DIGEST_LENGTH, 
    (const uint8_t *)RIOT_LABEL_ALIAS,lblSize(RIOT_LABEL_ALIAS))) != RIOT_SUCCESS){
        LogError("Failure: RiotCrypt_DeriveEccKey returned invalid status %d.", status);
        return status;
    }

    // Clean up potentially sensative data
    memset(cDigest, 0x00, RIOT_DIGEST_LENGTH);

    // Build the TBS (to be signed) region of Alias Key Certificate
    DERInitContext(&derCtx, derBuffer, DER_MAX_TBS);
    if ((status = X509GetAliasCertTBS(&derCtx, &x509AliasTBSData, &aliasKeyPub, &deviceIDPub, 
    FWID, RIOT_DIGEST_LENGTH)) != 0){
        LogError("Failure: X509GetAliasCertTBS returned invalid status %d.", status);
        return status;
    }

    // Sign the Alias Key Certificate's TBS region
    if ((status = RiotCrypt_Sign(&tbsSig, derCtx.Buffer, derCtx.Position, &deviceIDPriv)) != RIOT_SUCCESS){
        LogError("Failure: RiotCrypt_Sign returned invalid status %d.", status);
        return status;
    }

    // Generate Alias Key Certificate
    if ((status = X509MakeAliasCert(&derCtx, &tbsSig)) != 0){
        LogError("Failure: X509MakeAliasCert returned invalid status %d.", status);
        return status;
    }

    // Copy Alias Key Certificate
    length = sizeof(g_AKCert);
    if ((status = DERtoPEM(&derCtx, CERT_TYPE, g_AKCert, &length)) != 0){
        LogError("Failure: DERtoPEM returned invalid status %d.", status);
        return status;
    }
    g_AKCert[length] = '\0';
    g_AKCertLen = length;

    // Clean up potentially sensative data
    memset(FWID, 0, sizeof(FWID));

    // Copy DeviceID Public
    DERInitContext(&derCtx, derBuffer, DER_MAX_TBS);
    if ((status = X509GetDEREccPub(&derCtx, deviceIDPub)) != 0){
        LogError("Failure: X509GetDEREccPub returned invalid status %d.", status);
        return status;
    }
    length = sizeof(g_DeviceIDPub);
    if ((status = DERtoPEM(&derCtx, PUBLICKEY_TYPE, g_DeviceIDPub, &length)) != 0){
        LogError("Failure: DERtoPEM returned invalid status %d.", status);
        return status;
    }
    g_DeviceIDPub[length] = '\0';
    g_DeviceIDPubLen = length;

    // Copy Alias Key Pair
    DERInitContext(&derCtx, derBuffer, DER_MAX_TBS);
    if ((status = X509GetDEREcc(&derCtx, aliasKeyPub, aliasKeyPriv)) != 0){
        LogError("Failure: X509GetDEREcc returned invalid status %d.", status);
        return status;
    }
    length = sizeof(g_AliasKey);
    if ((status = DERtoPEM(&derCtx, ECC_PRIVATEKEY_TYPE, g_AliasKey, &length)) != 0){
        LogError("Failure: DERtoPEM returned invalid status %d.", status);
        return status;
    }
    g_AliasKey[length] = '\0';
    g_AliasKeyLen = length;

    // We don't do CSRs on this device, this should be true.
    if(SelfSignedDeviceCert) {
        // Build the TBS (to be signed) region of DeviceID Certificate
        DERInitContext(&derCtx, derBuffer, DER_MAX_TBS);
        if ((status = X509GetDeviceCertTBS(&derCtx, &x509DeviceTBSData, &deviceIDPub)) != 0){
            LogError("Failure: X509GetDeviceCertTBS returned invalid status %d.", status);
            return status;
        }

        // Sign the DeviceID Certificate's TBS region
        if ((status = RiotCrypt_Sign(&tbsSig, derCtx.Buffer, derCtx.Position, &deviceIDPriv)) != RIOT_SUCCESS){
            LogError("Failure: RiotCrypt_Sign returned invalid status %d.", status);
            return status;
        }

        // Generate DeviceID Certificate
        if ((status = X509MakeDeviceCert(&derCtx, &tbsSig)) != 0){
            LogError("Failure: X509MakeDeviceCert returned invalid status %d.", status);
            return status;
        }
        
        // DeviceID Cert type
        dcType = CERT_TYPE;
        } 
    else {
        // Initialize, create CSR TBS region
        DERInitContext(&derCtx, derBuffer, DER_MAX_TBS);
        if ((status = X509GetDERCsrTbs(&derCtx, &x509AliasTBSData, &deviceIDPub)) != 0){
            LogError("Failure: X509GetDERCsrTbs returned invalid status %d.", status);
            return status;
        }

        // Sign the Alias Key Certificate's TBS region
        if ((status = RiotCrypt_Sign(&tbsSig, derCtx.Buffer, derCtx.Position, &deviceIDPriv)) != RIOT_SUCCESS){
            LogError("Failure: RiotCrypt_Sign returned invalid status %d.", status);
            return status;
        }

        // Create CSR for DeviceID
        if ((status = X509GetDERCsr(&derCtx, &tbsSig)) != 0){
            LogError("Failure: X509GetDERCsr returned invalid status %d.", status);
            return status;
        }

        // DeviceID CSR type
        dcType = CERT_REQ_TYPE;
        }

    // Copy CSR/self-signed Cert
    length = sizeof(g_DICert);
    if ((status = DERtoPEM(&derCtx, dcType, g_DICert, &length)) != 0){
        LogError("Failure: DERtoPEM returned invalid status %d.", status);
        return status;
    }
    g_DICert[length] = '\0';
    g_DICertLen = length;
    
    // Clean up sensative data
    memset(&deviceIDPriv, 0, sizeof(RIOT_ECC_PRIVATE));

    // Display info for Alias Key Certificate
#if logging
    char *buf;
    buf = RIoTGetAliasCert(NULL);
    (void)printf("Alias Key Certificate\r\n%s\r\n", buf);
    buf = RIoTGetDeviceID(NULL);
    (void)printf("\r\nDeviceID Public\r\n%s\r\n", buf);
    buf = RIoTGetAliasKey(NULL);
    (void)printf("Alias Key Pair\r\n%s\r\n", buf);
    buf = RIoTGetDeviceCert(NULL);
    (void)printf("Device Certificate\r\n%s\r\n", buf);
#endif

    riot_enabled = true;
    return 0;
}
