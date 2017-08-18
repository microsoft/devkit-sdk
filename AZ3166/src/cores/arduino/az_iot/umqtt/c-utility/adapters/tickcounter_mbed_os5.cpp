// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "mbed.h"
#include "SystemTickCounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/tickcounter.h"

struct TICK_COUNTER_INSTANCE_TAG
{
    uint64_t current_ms;
};


TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE_TAG* result;
    result = new TICK_COUNTER_INSTANCE_TAG;
    result->current_ms = SystemTickCounterRead();
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
        delete tick_counter;
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    int result;
    if (tick_counter == NULL || current_ms == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        tick_counter->current_ms =  SystemTickCounterRead();
        *current_ms = (tickcounter_ms_t)tick_counter->current_ms;

        result = 0;
    }
    return result;
}
