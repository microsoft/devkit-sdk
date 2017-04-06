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

#ifndef __BUTTON_H_
#define __BUTTON_H_

#include "platform.h"
#include "platform_peripheral.h"


/** @addtogroup MICO_Drivers_interface
  * @{
  */
    
/** @defgroup MICO_keypad_Driver MiCO keypad Driver
  * @brief Provide driver interface for keypad devices
  * @{
  */


/** @addtogroup MICO_keypad_Driver
  * @{
  */
/** @defgroup MICO_Button_Driver MiCO Button Driver
  * @brief Provide driver interface for button
  * @{
  */


//--------------------------------  pin defines --------------------------------
typedef enum _button_index_e{
	IOBUTTON_EASYLINK = 0,
	IOBUTTON_USER_1,
	IOBUTTON_USER_2,
	IOBUTTON_USER_3,
	IOBUTTON_USER_4,
} button_index_e;

typedef void (*button_pressed_cb)(void) ;
typedef void (*button_long_pressed_cb)(void) ;

typedef struct _button_init_t{
	mico_gpio_t gpio;
	int long_pressed_timeout;
	button_pressed_cb pressed_func;
	button_long_pressed_cb long_pressed_func;
} button_init_t;

//------------------------------ user interfaces -------------------------------


/**
 * @brief Initialize button device.
 *
 * @param index: index of context
 * @param init: button_init_t struct
 *
 * @return none
 */
void button_init( int index, button_init_t init );

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#endif  // __BUTTON_H_
