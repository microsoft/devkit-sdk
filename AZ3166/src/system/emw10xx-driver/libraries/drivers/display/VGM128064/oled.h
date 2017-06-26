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

#ifndef __OLED_H_
#define __OLED_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "mico.h"
#include "platform.h"

/** @defgroup MICO_Drivers_interface MiCO Drivers Interface
  * @brief Provide driver interface for MiCO external devices
  * @{
  */

/** @addtogroup MICO_Drivers_interface
  * @{
  */

/** @defgroup MICO_display_Driver MiCO display Driver
  * @brief Provide driver interface for display devices
  * @{
  */

/** @addtogroup MICO_display_Driver
  * @{
  */

/** @defgroup MICO_OLED_Driver MiCO OLED Driver
  * @brief Provide driver interface for OLED
  * @{
  */

#ifndef OLED_SPI_PORT
#define OLED_SPI_PORT       (MICO_SPI_NONE)
#endif

#ifndef OLED_SPI_SCK
#define OLED_SPI_SCK        (MICO_GPIO_NONE)
#endif

#ifndef OLED_SPI_DIN
#define OLED_SPI_DIN        (MICO_GPIO_NONE)
#endif

#ifndef OLED_SPI_DC
#define OLED_SPI_DC         (MICO_GPIO_NONE)
#endif

#ifndef OLED_SPI_CS
#define OLED_SPI_CS         (MICO_GPIO_NONE)
#endif

#ifndef OLED_I2C_PORT
#define OLED_I2C_PORT       (MICO_I2C_NONE)
#endif

#define OLED_MODE 0
#define SIZE 16
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	

// typedef
#define u8     uint8_t
#define u16    uint16_t
#define u32    uint32_t

//----------------- OLED PIN ----------------  	
#define OLED_DC_INIT()     MicoGpioInitialize( (mico_gpio_t)OLED_SPI_DC, OUTPUT_PUSH_PULL )  // in case spi flash is wrote

#define OLED_CS_Clr()      MicoGpioOutputLow(OLED_SPI_CS)  //CS
#define OLED_CS_Set()      MicoGpioOutputHigh(OLED_SPI_CS)

#define OLED_DC_Clr()      MicoGpioOutputLow(OLED_SPI_DC)  //DC
#define OLED_DC_Set()      MicoGpioOutputHigh(OLED_SPI_DC)

#define OLED_RST_Clr()
#define OLED_RST_Set()

//PC0~7,��Ϊ������
#define DATAOUT(x)           GPIO_Write(GPIOC,x);//���  
//ʹ��4�ߴ��нӿ�ʱʹ�� 

#define OLED_SCLK_Clr() MicoGpioOutputLow(OLED_SPI_SCK)  //CLK
#define OLED_SCLK_Set() MicoGpioOutputHigh(OLED_SPI_SCK)

#define OLED_SDIN_Clr() MicoGpioOutputLow(OLED_SPI_DIN)  //DIN
#define OLED_SDIN_Set() MicoGpioOutputHigh(OLED_SPI_DIN)
	     
#define OLED_CMD  0	//д����
#define OLED_DATA 1	//д����

//-------------------------------- display define ------------------------------
// for 8*16 char can only display 4 rows, 16 chars each row.
#define OLED_DISPLAY_ROW_1    0    // yellow
#define OLED_DISPLAY_ROW_2    2    // blue
#define OLED_DISPLAY_ROW_3    4    // blue
#define OLED_DISPLAY_ROW_4    6    // blue

#define OLED_DISPLAY_COLUMN_START    0    // colloum from left pos 0

#define OLED_DISPLAY_MAX_CHAR_PER_ROW    16   // max 16 chars each row


//OLED�����ú���
void OLED_WR_Byte(u8 dat, u8 cmd);   
void OLED_WR_Bytes(u8 *dat, u8 len, u8 cmd);   
void OLED_Display_On(void);
void OLED_Display_Off(void);	  

void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y,u8 chr);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);

void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(u8 x,u8 y,u8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);


/*-------------------------------------------------------- USER INTERFACES -----------------------------------------*/


/**
 * @brief Initialize OLED device.
 *
 * @return none
 */
void OLED_Init(void);



/**
 * @brief Clear up all data shown on OLED
 *
 * @return none
 */
void OLED_Clear(void);


/**
 * @brief show string in OLED specified place
 *
 * @param x: Position the X axis of the stiring to display
 * @param y: Position the Y axis of the string to display
 * @param p: String to be displayed in OLED
 * 
 * @return none
 */
void OLED_ShowString(u8 x,u8 y, char *p);
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

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  
	 



