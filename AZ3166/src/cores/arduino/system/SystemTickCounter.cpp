// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "mbed.h"
#include "SystemTickCounter.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The tick counter from mbed OS will be overflow (go back to zero) after approximately 70 minutes (4294s).
// So here extend the  tick counter 64bit.
static Ticker cycle_ticker;
static volatile uint32_t last_ticker_us = 0;
static volatile uint64_t long_ticker_ms = 0;

static void cycle_accumulator(void)
{
    SystemTickCounterRead();
}

void SystemTickCounterInit(void)
{
    cycle_ticker.attach(cycle_accumulator, 60.0);
}

uint64_t SystemTickCounterRead(void)
{
    uint64_t result;
    uint32_t t;

    core_util_critical_section_enter();
    t = us_ticker_read();
    if (t < last_ticker_us)
    {
        long_ticker_ms += 0xFFFFFFFF / 1000;
    }
    last_ticker_us = t;

    result = long_ticker_ms + t / 1000; 
    core_util_critical_section_exit();

    return result;
}
