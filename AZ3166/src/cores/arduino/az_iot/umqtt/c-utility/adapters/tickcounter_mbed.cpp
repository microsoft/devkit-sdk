// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "mbed.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/tickcounter.h"

struct TICK_COUNTER_INSTANCE_TAG
{
    tickcounter_ms_t current_ms;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The tick counter from mbed OS will be overflow (go back to zero) after approximately 70 minutes (4294s).
// So here externd the  tick counter 64bit.
static Ticker cycle_ticker;
static volatile uint32_t last_ticker_us = 0;
static volatile uint64_t long_ticker_ms = 0;

static uint64_t _ms_ticker_read(void)
{
    uint64_t result;
    uint32_t t = us_ticker_read();

    core_util_critical_section_enter();
    if (t < last_ticker_us)
    {
        long_ticker_ms += 0xFFFFFFFF / 1000;
    }
    last_ticker_us = t;

    result = long_ticker_ms + t / 1000; 
    core_util_critical_section_exit();

    return result;
}

static void cycle_accumulator(void)
{
    _ms_ticker_read();
}

void tickcounter_init(void)
{
    cycle_ticker.attach(cycle_accumulator, 60.0);
}

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE_TAG* result;
    result = new TICK_COUNTER_INSTANCE_TAG;
    result->current_ms = _ms_ticker_read();
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
        delete tick_counter;
    }
}

tickcounter_ms_t tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    int result;
    if (tick_counter == NULL || current_ms == NULL)
    {
        result = __FAILURE__;
    }
    else
    {
        tick_counter->current_ms =  _ms_ticker_read();
        *current_ms = tick_counter->current_ms;

        result = 0;
    }
    return result;
}
