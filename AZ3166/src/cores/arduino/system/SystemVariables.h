// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef __SYSTEM_VARIABLES_H__
#define __SYSTEM_VARIABLES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TARGET_AZ3166

#define BOARD_NAME "AZ3166"
#define BOARD_FULL_NAME "MXChip IoT DevKit"
#define BOARD_MCU "STM32F412"

#endif

int GetMACWithoutColon(char* buff);

const char* GetBoardID(void);

#ifdef __cplusplus
}
#endif

#endif  // __SYSTEM_VARIABLES_H__
