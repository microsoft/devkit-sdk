/*(Copyright)

Microsoft Copyright 2017
Confidential Information

*/

#include "DiceCore.h"
#include "DiceSha256.h"
#include <stdio.h>
//#include "stm32f4xx_hal.h"
//#include "stm32f4xx_nucleo.h"

// Protected data
extern DICE_UDS             DiceUDS;                        // NV
extern DICE_CMPND_ID        DiceCDI;                        // V
extern DICE_SHA256_CONTEXT  DiceHashCtx;                    // V
extern uint8_t              vDigest[DICE_DIGEST_LENGTH];    // V
extern uint8_t              rDigest[DICE_DIGEST_LENGTH];    // V
extern void* __start_riot_core;
extern void* __stop_riot_core;

// Non-protected data
extern DICE_DATA            DiceData;

// Prototypes
static uint32_t _DiceMeasure(uint8_t *data, size_t dataSize, uint8_t *digest, size_t digestSize);
static uint32_t _DiceDeriveCDI(uint8_t *digest, size_t digestLen);

static void _BZERO(void *p, uint32_t l);

// Functions

void
DiceCore(
    void
)
{
    // Disable FW PreArm in protected code
	//__HAL_FIREWALL_PREARM_DISABLE();
	
    // Compute digest of RIoT Core
    if (_DiceMeasure(DiceData.riotCore, DiceData.riotSize, rDigest, DICE_DIGEST_LENGTH)) {
        // Enter remediation
        goto Remediate;
    }
		
    // Derive CDI based on measurement of RIoT Core and UDS.
   if (_DiceDeriveCDI(rDigest, DICE_DIGEST_LENGTH)) {
       // Enter remediation
       goto Remediate;
   }

    // Clean up potentially sensative data
    _BZERO(vDigest, DICE_DIGEST_LENGTH);
    _BZERO(rDigest, DICE_DIGEST_LENGTH);
    _BZERO(&DiceHashCtx, sizeof(DiceHashCtx));

    // The CDI is ready and UDS digest is cleaned up.
    // Enable shared access to volatile data.
    //__HAL_FIREWALL_VOLATILEDATA_SHARED_ENABLE();

    // Set FPA bit for proper FW closure when exiting protected code
    //__HAL_FIREWALL_PREARM_ENABLE();

    // We're done.
    return;

Remediate:
    // Set FPA bit for proper FW closure when exiting protected code
    //__HAL_FIREWALL_PREARM_ENABLE();

    // Access to volatile data outside protected code is our indication
    // of success.  We will enter remediation upon return from DiceCore.
    return;
}

static uint32_t
_DiceMeasure(
    uint8_t    *data,
    size_t      dataSize,
    uint8_t    *digest,
    size_t      digestSize
)
{
    // Validate parameters
    if ((!data) || (dataSize < (2 * sizeof(uint32_t))) ||
        (!digest) || (digestSize != DICE_DIGEST_LENGTH)) {
        // Remediation
        return 1;
    }

    // Measure data area
    DiceSHA256(data, dataSize, digest);

    // Success
    return 0;
}

static uint32_t
_DiceDeriveCDI(
    uint8_t    *digest,
    size_t      digestSize
)
{
    // Validate parameter
    if (!(digest) || (digestSize != DICE_DIGEST_LENGTH)) {
        // Remediate
        goto Error;
    }

    // Don't use the UDS directly.
    DiceSHA256(DiceUDS.bytes, DICE_UDS_LENGTH, vDigest);

    // Derive CDI value based on UDS and RIoT Core measurement
    DiceSHA256_2(vDigest, DICE_DIGEST_LENGTH,
                      rDigest, DICE_DIGEST_LENGTH,
                      DiceCDI.bytes);

    // Success
    return 0;

Error:
    // Failure
    return 1;
}

static void _BZERO(void *p, uint32_t l)
{
    int i;
    for (i = 0; i < (l); i++) {
        ((uint8_t*)p)[i] = 0;
    }
}

