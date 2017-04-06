/* MiCO Team
 * Copyright (c) 2017 MXCHIP Information Tech. Co.,Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PLATFORM_ASSERT_h__
#define __PLATFORM_ASSERT_h__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/


#if defined ( __GNUC__ ) && !defined(__CC_ARM)

#if defined ( __clang__ )

static inline /*@noreturn@*/void MICO_TRIGGER_BREAKPOINT( void ) __attribute__((analyzer_noreturn))
{
    __asm__("bkpt");
}

#else

#define MICO_TRIGGER_BREAKPOINT( ) do { __asm__("bkpt"); } while (0)

#endif /* #if defined ( __clang__ ) */

#define MICO_ASSERTION_FAIL_ACTION() MICO_TRIGGER_BREAKPOINT()

#ifdef NO_VECTORS
#define MICO_DISABLE_INTERRUPTS() do { __asm__("" : : : "memory"); } while (0)
#define MICO_ENABLE_INTERRUPTS() do { __asm__("" : : : "memory"); } while (0)
#else
#define MICO_DISABLE_INTERRUPTS() do { __asm__("CPSID i" : : : "memory"); } while (0)
#define MICO_ENABLE_INTERRUPTS() do { __asm__("CPSIE i" : : : "memory"); } while (0)
#endif

#define CSPR_INTERRUPTS_DISABLED (0x80)

#elif defined ( __IAR_SYSTEMS_ICC__ )

#define MICO_TRIGGER_BREAKPOINT() do { __asm("bkpt 0"); } while (0)

#define MICO_DISABLE_INTERRUPTS() do { __asm("CPSID i"); } while (0)
#define MICO_ENABLE_INTERRUPTS() do { __asm("CPSIE i"); } while (0)

#define MICO_ASSERTION_FAIL_ACTION() MICO_TRIGGER_BREAKPOINT()

#elif defined ( __CC_ARM )
#define MICO_TRIGGER_BREAKPOINT()   while (0)
#define MICO_ASSERTION_FAIL_ACTION()  MICO_TRIGGER_BREAKPOINT()

#endif

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*"C" */
#endif

#endif
