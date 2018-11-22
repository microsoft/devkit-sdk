// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#include "mbed.h"
#include "SystemTickCounter.h"
#include "azure_c_shared_utility/tickcounter.h"

static TICK_COUNTER_HANDLE tick_counter = NULL;

void SystemTickCounterInit(void)
{
    tick_counter = tickcounter_create();
}

uint64_t SystemTickCounterRead(void)
{
    uint64_t result;

    tickcounter_get_current_ms(tick_counter, &result);

    return result;
}
