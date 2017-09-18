/*(Copyright)

Microsoft Copyright 2017
Confidential Information

*/
#include "DiceInit.h"
#include "DiceCore.h"
#include "RiotCore.h"
#include "iothub_client_dps.h"
#include <stdio.h>

// Addresses of riot core code
extern void* __start_riot_core;
extern void* __stop_riot_core;

// Protected data
extern DICE_UDS      DiceUDS;                       // NV
extern DICE_CMPND_ID DiceCDI;                       // V
extern uint8_t       vDigest[DICE_DIGEST_LENGTH];   // V
extern uint8_t       rDigest[DICE_DIGEST_LENGTH];   // V

// Non-protected data
DICE_DATA DiceData = { 0 };

// Functions
void DiceRemediate(void)
{
    // TODO: FORCE RESET HERE
    while (1);
}

int StartDiceInit(void)
{
    int result = 0;
    (void)printf("The riot_core start address: %p\r\n", &__start_riot_core);
    (void)printf("The riot_core end address: %p\r\n", &__stop_riot_core);

    // Initialize DICE
    DiceInit();

    // Launch protected DICE code. This will measure RIoT Core, derive the
    // CDI value. It must execute with interrupts disabled. Therefore, it
    // must return so we can restore interrupt state.
    DiceCore();

    // If DiceCore detects an error condition, it will not enable access to
    // the volatile storage segment. This attempt to transfer control to RIoT
    // will trigger a system reset. We will not be able to proceed.
    // TODO: DETECT WHEN A RESET HAS OCCURRED AND TAKE SOME ACTION.
    RiotStart(DiceCDI.bytes, (uint16_t)DICE_DIGEST_LENGTH);

    // Transfer control to firmware
    result = IoTHubClientStart();

    return result;
}

void DiceInit(void)
{
    // Up-front sanity check
    if (DiceUDS.tag != DICE_UDS_TAG) {
        // We must be provisioned.
        DiceRemediate();
    }

    // Initialize CDI structure
    memset(&DiceCDI, 0x00, sizeof(DICE_CMPND_ID));
    DiceCDI.tag = DICE_CMPND_TAG;

    // Pointers to protected DICE Data
    DiceData.UDS = &DiceUDS;
    DiceData.CDI = &DiceCDI;

    // Start of RIoT Invariant Code
    DiceData.riotCore = (uint8_t*)&__start_riot_core;

    // Calculate size of RIoT Core
    DiceData.riotSize = (uint8_t*)&__stop_riot_core - DiceData.riotCore;
    (void)printf(">>> DiceData.riotSize == %d.\r\n", DiceData.riotSize);

    return;
}
