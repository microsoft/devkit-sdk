// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "mbed.h"
#include "mico.h"
#include "SystemFunc.h"
#include "SystemWeb.h"

void SystemReboot(void)
{
    mico_system_reboot();
}

void SystemStandby(int timeout)
{
    MicoSystemStandBy(timeout);
}


WEAK void __sys_setup(void)
{
    // Default only enable wifi setting
    EnableSystemWeb(0);
}