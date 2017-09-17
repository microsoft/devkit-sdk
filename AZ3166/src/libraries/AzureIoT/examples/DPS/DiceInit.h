/*(Copyright)

Microsoft Copyright 2017
Confidential Information

*/
#ifndef __DICE_INIT_H
#define __DICE_INIT_H

//#include "stm32f4xx_hal.h"
//#include "stm32f4xx_nucleo.h"

//
// For MCU initializaion
//

//#if !defined  (HSE_VALUE)
//#define HSE_VALUE    ((uint32_t)8000000) /*!< Value of the External oscillator in Hz */
//#endif /* HSE_VALUE */

//#if !defined  (MSI_VALUE)
//#define MSI_VALUE    ((uint32_t)4000000) /*!< Value of the Internal oscillator in Hz*/
//#endif /* MSI_VALUE */

//#if !defined  (HSI_VALUE)
//#define HSI_VALUE    ((uint32_t)16000000) /*!< Value of the Internal oscillator in Hz*/
//#endif /* HSI_VALUE */

//#define VECT_TAB_OFFSET  0x00

//
// Prototypes
//
//void Error_Handler(void);

void DiceInit(void);
void DiceRemediate(void);
int StartDiceInit(void);

#endif
