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

#ifndef __PlatformLogging_h__
#define __PlatformLogging_h__

#include "mico_common.h"
#include "mico_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MICO_PLATFORM_LOG_INFO_DISABLE
	#define platform_log(M, ...) custom_log("Platform", M, ##__VA_ARGS__)
	#define platform_log_trace() custom_log_trace("Platform")
#else
	#define platform_log(M, ...) 
	#define platform_log_trace() 
#endif

#ifdef __cplusplus
} /*"C" */
#endif

#endif // __PlatformLogging_h__

