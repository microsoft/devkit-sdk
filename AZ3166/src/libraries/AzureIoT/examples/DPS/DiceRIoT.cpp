/*(Copyright)

Microsoft Copyright 2017
Confidential Information

*/
#include "DiceRIoT.h"
#include "DiceCore.h"
#include "RiotCore.h"
#include <stdio.h>

// attribute section info from AZ3166.ld
extern void* __start_riot_core;
extern void* __stop_riot_core;

// Protected data
extern DICE_UDS DiceUDS;
extern DICE_CMPND_ID DiceCDI;

// Non-protected data
DICE_DATA DiceData = { 0 };
DICE_CMPND_ID DiceCDI = { DICE_CMPND_TAG , { 0x00 } };

int DiceRIoTStart(void)
{
    (void)printf("The riot_core start address: %p\r\n", &__start_riot_core);
    (void)printf("The riot_core end address: %p\r\n", &__stop_riot_core);

    // Initialize DICE
    if (DiceInit() != 0){
        return -1;
    }

    // Launch protected DICE code. This will measure RIoT Core, derive the
    // CDI value. It must execute with interrupts disabled. Therefore, it
    // must return so we can restore interrupt state.
    if (DiceCore() != 0){
        return -1;
    }

    // If DiceCore detects an error condition, it will not enable access to
    // the volatile storage segment. This attempt to transfer control to RIoT
    // will trigger a system reset. We will not be able to proceed.
    // TODO: DETECT WHEN A RESET HAS OCCURRED AND TAKE SOME ACTION.
    if (RiotStart(DiceCDI.bytes, (uint16_t)DICE_DIGEST_LENGTH) != 0){
        return -1;
    }

    return 0;
}

int DiceInit(void)
{
    // Up-front sanity check
    if (DiceUDS.tag != DICE_UDS_TAG) {
        return -1;
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
    if((DiceData.riotSize = (uint8_t*)&__stop_riot_core - DiceData.riotCore) == 0){
        return -1;
    }

    return 0;
}
