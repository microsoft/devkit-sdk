// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "mico.h"
#include "SystemFunc.h"

void SystemReboot(void)
{
    mico_system_reboot();
}

void SystemStandby(int timeout)
{
    MicoSystemStandBy(timeout);
}