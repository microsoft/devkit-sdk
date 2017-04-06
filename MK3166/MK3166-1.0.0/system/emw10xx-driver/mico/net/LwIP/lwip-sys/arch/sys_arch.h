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


#ifndef __SYS_RTXC_H__
#define __SYS_RTXC_H__

#include "mico_rtos.h"

#define SYS_MBOX_NULL ((mico_queue_t)0)
#define SYS_SEM_NULL  ((mico_semaphore_t)0)

typedef mico_semaphore_t  sys_sem_t;
typedef mico_queue_t      sys_mbox_t;
typedef mico_thread_t     sys_thread_t;

#endif /* __SYS_RTXC_H__ */

