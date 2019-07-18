// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#undef MOCKABLE_FUNCTION
#undef MOCKABLE_FUNCTION_WITH_RETURNS

/* This header is meant to be included by production code headers, so that the MOCKABLE_FUNCTION gets enabled. */
/* 
    If you are porting to a new platform and do not want to build the tests, but only the production code,
    simply make sure that this file is in the include path (either by copying it to your inc folder or 
    by adjusting the include paths).
*/

#define DO_NOTHING_WITH_RETURN_VALUES(success_return_value, failure_return_value) \

/* These 2 macros are used to check if a type is "void" or not */
#define UMOCK_C_PROD_TEST_void 0
#define UMOCK_C_PROD_IS_NOT_VOID(x) \
    MU_IF(MU_C2(UMOCK_C_PROD_TEST_,x), 1, 0)

#ifdef ENABLE_MOCKS

#ifdef ENABLE_MOCK_FILTERING
#define ENABLE_MOCK_FILTERING_SWITCH 1
#else
#define ENABLE_MOCK_FILTERING_SWITCH 0
#endif

#include "azure_macro_utils/macro_utils.h"

#define UMOCK_C_PROD_ARG_IN_SIGNATURE(count, arg_type, arg_name) arg_type arg_name MU_IFCOMMA(count)

#define MOCKABLE_FUNCTION_DISABLED(do_returns, modifiers, result, function, ...) \
    result modifiers function(MU_IF(MU_COUNT_ARG(__VA_ARGS__), , void) MU_FOR_EACH_2_COUNTED(UMOCK_C_PROD_ARG_IN_SIGNATURE, __VA_ARGS__)); \
    MU_IF(do_returns, MU_IF(UMOCK_C_PROD_IS_NOT_VOID(result), DO_NOTHING_WITH_RETURN_VALUES,), )

/* Codes_SRS_UMOCK_C_LIB_01_001: [MOCKABLE_FUNCTION shall be used to wrap function definition allowing the user to declare a function that can be mocked.]*/
#define MOCKABLE_FUNCTION(modifiers, result, function, ...) \
    MU_IF(ENABLE_MOCK_FILTERING_SWITCH,MU_IF(MU_C2(please_mock_, function),MOCKABLE_FUNCTION_DISABLED,MOCKABLE_FUNCTION_UMOCK_INTERNAL_WITH_MOCK), MOCKABLE_FUNCTION_UMOCK_INTERNAL_WITH_MOCK) (0, modifiers, result, function, __VA_ARGS__)

/* Codes_SRS_UMOCK_C_LIB_01_212: [ `MOCKABLE_FUNCTION_WITH_RETURNS` shall be used to wrap function definitions, allowing the user to declare a function that can be mocked and aditionally declares the values that are to be returned in case of success and failure. ]*/
#define MOCKABLE_FUNCTION_WITH_RETURNS(modifiers, result, function, ...) \
    MU_IF(ENABLE_MOCK_FILTERING_SWITCH,MU_IF(MU_C2(please_mock_, function),MOCKABLE_FUNCTION_DISABLED,MOCKABLE_FUNCTION_UMOCK_INTERNAL_WITH_MOCK), MOCKABLE_FUNCTION_UMOCK_INTERNAL_WITH_MOCK) (1, modifiers, result, function, __VA_ARGS__)

#include "umock_c/umock_c.h"

#else

#include "azure_macro_utils/macro_utils.h"

#define UMOCK_C_PROD_ARG_IN_SIGNATURE(count, arg_type, arg_name) arg_type arg_name MU_IFCOMMA(count)

/* Codes_SRS_UMOCK_C_LIB_01_002: [The macro shall generate a function signature in case ENABLE_MOCKS is not defined.] */
/* Codes_SRS_UMOCK_C_LIB_01_005: [If ENABLE_MOCKS is not defined, MOCKABLE_FUNCTION shall only generate a declaration for the function.] */
/* Codes_SRS_UMOCK_C_LIB_01_001: [MOCKABLE_FUNCTION shall be used to wrap function definition allowing the user to declare a function that can be mocked.]*/
#define MOCKABLE_FUNCTION(modifiers, result, function, ...) \
    result modifiers function(MU_IF(MU_COUNT_ARG(__VA_ARGS__),,void) MU_FOR_EACH_2_COUNTED(UMOCK_C_PROD_ARG_IN_SIGNATURE, __VA_ARGS__));

/* Codes_SRS_UMOCK_C_LIB_01_213: [ The macro shall generate a function signature in case `ENABLE_MOCKS` is not defined. ]*/
/* Codes_SRS_UMOCK_C_LIB_01_212: [ `MOCKABLE_FUNCTION_WITH_RETURNS` shall be used to wrap function definitions, allowing the user to declare a function that can be mocked and aditionally declares the values that are to be returned in case of success and failure. ]*/
#define MOCKABLE_FUNCTION_WITH_RETURNS(modifiers, result, function, ...) \
    result modifiers function(MU_IF(MU_COUNT_ARG(__VA_ARGS__),,void) MU_FOR_EACH_2_COUNTED(UMOCK_C_PROD_ARG_IN_SIGNATURE, __VA_ARGS__)); \
    MU_IF(UMOCK_C_PROD_IS_NOT_VOID(result), DO_NOTHING_WITH_RETURN_VALUES,)

#endif
