/*
    mbed Memory Status Helper
    Copyright (c) 2017 Max Vilimpoc
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

/**
 * Purpose: Print out thread info and other useful details using 
 *          only raw serial access.
 * 
 * Based on mbed_board.c's error printing functionality, minus 
 * pulling in all the printf() code.
 */

#include "platform/critical.h"
#include "platform/mbed_stats.h"

#ifndef DEBUG_ISR_STACK_USAGE
#define DEBUG_ISR_STACK_USAGE  0
#endif
#ifndef DEBUG_MEMORY_CONTENTS
#define DEBUG_MEMORY_CONTENTS  0
#endif

#define OUTPUT_SERIAL          1
#define OUTPUT_RTT             0
#define OUTPUT_SWO             0

#if DEBUG_ISR_STACK_USAGE
#include "compiler_abstraction.h"

// Value is sprayed into all of the ISR stack at boot time.
static const uint32_t ISR_STACK_CANARY = 0xAFFEC7ED; // AFFECTED

// Refers to linker script defined symbol, may not be available
// on all platforms.
extern uint32_t __StackLimit;
extern uint32_t __StackTop;

void fill_isr_stack_with_canary(void)
{
    uint32_t * bottom = &__StackLimit;
    uint32_t * top    = (uint32_t *) GET_SP();

    for (; bottom < top; bottom++)
    {
        *bottom = ISR_STACK_CANARY;
    }
}
#endif // DEBUG_ISR_STACK_USAGE

#if OUTPUT_SERIAL && DEVICE_SERIAL
#include "hal/serial_api.h"

extern int      stdio_uart_inited;
extern serial_t stdio_uart;

static void output_serial_init(void)
{
    if (!stdio_uart_inited) 
    {
        serial_init(&stdio_uart, STDIO_UART_TX, STDIO_UART_RX);
        serial_baud(&stdio_uart, 115200); // This is hard coded.
    }
}

static void output_serial_print_label(const char * label)
{
    core_util_critical_section_enter();
    output_serial_init();

    while (*label) serial_putc(&stdio_uart, *label++);
    
    core_util_critical_section_exit();
}
#endif

#if OUTPUT_RTT
#include "RTT/SEGGER_RTT.h"

enum
{
    DEFAULT_RTT_UP_BUFFER = 0
};

static void output_rtt_init(void)
{
    static int initialized = 0;
    
    if (!initialized)
    {
        SEGGER_RTT_ConfigUpBuffer(DEFAULT_RTT_UP_BUFFER, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
        
        initialized = 1;
    }
}

static void output_rtt_print_label(const char * label)
{
    output_rtt_init();
    SEGGER_RTT_WriteString(DEFAULT_RTT_UP_BUFFER, label);
}
#endif

#if OUTPUT_SWO
#ifdef NRF52
#include "nrf.h"
#endif

static void output_swo_init(void)
{
    static int initialized = 0;
    
    if (!initialized)
    {
        NRF_CLOCK->TRACECONFIG = (NRF_CLOCK->TRACECONFIG & ~CLOCK_TRACECONFIG_TRACEPORTSPEED_Msk) |
            (CLOCK_TRACECONFIG_TRACEPORTSPEED_4MHz << CLOCK_TRACECONFIG_TRACEPORTSPEED_Pos);

        ITM->TCR |= 1;
        ITM->TER |= 1;
        
        initialized = 1;
    }
}

static void output_swo_print_label(const char * label)
{
    output_swo_init();
    while (*label) ITM_SendChar(*label++);
}
#endif

static void nway_print_label(const char * label)
{
#if OUTPUT_SERIAL
    output_serial_print_label(label);
#endif

#if OUTPUT_RTT
    output_rtt_print_label(label);
#endif
    
#if OUTPUT_SWO
    output_swo_print_label(label);
#endif
}

static const char HEX[] = "0123456789ABCDEF";

static void debug_print_u32(uint32_t u32)
{
    char output[9] = {0};
    
    // Always printed as big endian.
    output[0] = HEX[(((uint32_t) u32 & 0xf0000000) >> 28)];
    output[1] = HEX[(((uint32_t) u32 & 0x0f000000) >> 24)];
    output[2] = HEX[(((uint32_t) u32 & 0x00f00000) >> 20)];
    output[3] = HEX[(((uint32_t) u32 & 0x000f0000) >> 16)];
    output[4] = HEX[(((uint32_t) u32 & 0x0000f000) >> 12)];
    output[5] = HEX[(((uint32_t) u32 & 0x00000f00) >>  8)];
    output[6] = HEX[(((uint32_t) u32 & 0x000000f0) >>  4)];
    output[7] = HEX[(((uint32_t) u32 & 0x0000000f) >>  0)];
    
    nway_print_label(output);
}

static void debug_print_pointer(const void * pointer)
{
    debug_print_u32((uint32_t) pointer);
}

#define DPL(X) nway_print_label((X))

#if (defined (MBED_CONF_RTOS_PRESENT) && (MBED_CONF_RTOS_PRESENT != 0))
#include "cmsis_os.h"

// Temporarily #undef NULL or the compiler complains about previous def.
#undef NULL
#include "rt_TypeDef.h"
 
// No public forward declaration for this.
extern P_TCB rt_tid2ptcb (osThreadId thread_id);

static void print_thread_info(osThreadId threadId)
{
    if (!threadId) return;

    osEvent event;
    
    P_TCB tcb = rt_tid2ptcb(threadId);
    
    DPL("    stack ( start: ");
    debug_print_pointer(tcb->stack);
    
    event = _osThreadGetInfo(threadId, osThreadInfoStackSize);
    
    DPL(" end: ");
    debug_print_pointer(((uint8_t *) tcb->stack + event.value.v)); // (tcb->priv_stack)));

    DPL(" size: ");
    debug_print_u32(event.value.v);
    
    event = _osThreadGetInfo(threadId, osThreadInfoStackMax);
    DPL(" used: ");
    debug_print_u32(event.value.v);

    
    DPL(" ) ");

    DPL("thread ( id: ");
    debug_print_pointer(threadId);
    
    event = _osThreadGetInfo(threadId, osThreadInfoEntry);
    DPL(" entry: ");
    debug_print_pointer(event.value.p);

    DPL(" )\r\n");
}

void print_all_thread_info(void)
{
    osThreadEnumId enumId   = _osThreadsEnumStart();
    osThreadId     threadId = NULL;

    while ((threadId = _osThreadEnumNext(enumId)))
    {
        print_thread_info(threadId);
    }
 
    _osThreadEnumFree(enumId);
}

void print_current_thread_id(void)
{
    DPL("Current thread: ");
    debug_print_pointer(osThreadGetId());
    DPL("\r\n");
}
#endif // MBED_CONF_RTOS_PRESENT

#if DEBUG_MEMORY_CONTENTS
static void print_memory_contents(const uint32_t * start, const uint32_t * end)
{
    uint8_t line = 0;
    
    for (; start < end; start++)
    {
        if (0 == line)
        {
            debug_print_pointer(start);
            DPL(": ");
        }
        
        debug_print_u32(*start);
        
        line++;
        
        if (16 == line)
        {
            DPL("\r\n");
            line = 0;
        }
    }
}
#endif

extern uint32_t mbed_stack_isr_size;

#if DEBUG_ISR_STACK_USAGE
uint32_t calculate_isr_stack_usage(void)
{
    for (const uint32_t * stack = &__StackLimit; stack < &__StackTop; stack++)
    {
        if (*stack != ISR_STACK_CANARY)
        {
            return (uint32_t) &__StackTop - (uint32_t) stack;
        }
    }
    
    return mbed_stack_isr_size;
}
#endif

void print_heap_info(void)
{
    extern unsigned char * mbed_heap_start;
    extern uint32_t        mbed_heap_size;
    
    mbed_stats_heap_t      heap_stats;
    
    mbed_stats_heap_get(&heap_stats);

    DPL(">>heap ( start: ");
    debug_print_pointer(mbed_heap_start);
    
    DPL(" end: ");
    debug_print_pointer(mbed_heap_start + mbed_heap_size);
    
    DPL(" size: ");
    debug_print_u32(mbed_heap_size);
    
    DPL(" used: ");
    debug_print_u32(heap_stats.max_size);
    
    DPL(" )  alloc ( ok: ");
    debug_print_u32(heap_stats.alloc_cnt);
    
    DPL("  fail: ");
    debug_print_u32(heap_stats.alloc_fail_cnt);
    
    DPL(" )\r\n");
    
#if DEBUG_MEMORY_CONTENTS
    // Print ISR stack contents.
    print_memory_contents(&__StackLimit, &__StackTop);
#endif
}

void print_isr_stack_info(void)
{
    extern unsigned char * mbed_stack_isr_start;
    
    DPL(">>isr_stack ( start: ");
    debug_print_pointer(mbed_stack_isr_start);
    
    DPL(" end: ");
    debug_print_pointer(mbed_stack_isr_start + mbed_stack_isr_size);
    
    DPL(" size: ");
    debug_print_u32(mbed_stack_isr_size);
    
#if DEBUG_ISR_STACK_USAGE
    DPL(" used: ");
    debug_print_u32(calculate_isr_stack_usage());
#endif

    DPL(" )\r\n");

#if DEBUG_MEMORY_CONTENTS
    // Print ISR stack contents.
    print_memory_contents(&__StackLimit, &__StackTop);
#endif
}

void print_heap_and_isr_stack_info(void)
{
    print_heap_info();
    print_isr_stack_info();
}
