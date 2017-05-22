/**
******************************************************************************
* @file    fatfs_diskio.c 
* @author  Ding QQ
* @version V1.0.0
* @date    21-May-2015
* @brief   MiCO RTOS disk io control.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "mico.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Block Size in Bytes */
#define BLOCK_SIZE                8
#define SECTOR_SIZE               512
//#define SECTOR_COUNT              2048
#define FLASH_SECTOR              4096

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
static void MicoFlashEraseWrite( mico_partition_t partition, volatile uint32_t* off_set, uint8_t* data_addr ,uint32_t size );
mico_logic_partition_t *fatfs_partition;

/* Private function prototypes -----------------------------------------------*/
DSTATUS SFLASHDISK_initialize (void);
DSTATUS SFLASHDISK_status (void);
DRESULT SFLASHDISK_read (BYTE*, DWORD, BYTE);
#if _USE_WRITE == 1
  DRESULT SFLASHDISK_write (const BYTE*, DWORD, BYTE);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT SFLASHDISK_ioctl (BYTE, void*);
#endif /* _USE_IOCTL == 1 */
  
Diskio_drvTypeDef  SFLASHDISK_Driver =
{
  SFLASHDISK_initialize,
  SFLASHDISK_status,
  SFLASHDISK_read, 
#if  _USE_WRITE
  SFLASHDISK_write,
#endif  /* _USE_WRITE == 1 */  
#if  _USE_IOCTL == 1
  SFLASHDISK_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  None
  * @retval DSTATUS: Operation status
  */
DSTATUS SFLASHDISK_initialize(void)
{
  Stat = STA_NOINIT;
  fatfs_partition = MicoFlashGetInfo( MICO_PARTITION_FILESYS );
  Stat &= ~STA_NOINIT;
  return RES_OK;
}

/**
  * @brief  Gets Disk Status
  * @param  None
  * @retval DSTATUS: Operation status
  */
DSTATUS SFLASHDISK_status(void)
{
  Stat = STA_NOINIT;
  
  Stat &= ~STA_NOINIT;

  return RES_OK;
}

/**
  * @brief  Reads Sector(s)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT SFLASHDISK_read(BYTE *buff, DWORD sector, BYTE count)
{
  DRESULT res = RES_OK;
  uint32_t offset;

  for(; count>0; count--)
  {
    offset = (uint32_t)sector*SECTOR_SIZE;
    MicoFlashRead( MICO_PARTITION_FILESYS, &offset, (uint8_t *)buff, SECTOR_SIZE);
    sector++;
    buff += SECTOR_SIZE;
  }
  return res;
}

/**
  * @brief  Writes Sector(s)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT SFLASHDISK_write(const BYTE *buff, DWORD sector, BYTE count)
{ 
  DRESULT res = RES_OK;
  uint32_t offset;

  for(; count>0; count--)
  {
    offset = (uint32_t)sector*SECTOR_SIZE;
    MicoFlashEraseWrite( MICO_PARTITION_FILESYS, &offset, (uint8_t *)buff, SECTOR_SIZE);
    sector++;
    buff += SECTOR_SIZE;
  }
  
  return res;
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT SFLASHDISK_ioctl(BYTE cmd, void *buff)
{
  DRESULT res = RES_OK;
  DWORD nFrom,nTo;
  int i;
  
  if (Stat & STA_NOINIT) return RES_NOTRDY;
//  char *buf = buff;
  
  switch (cmd)
  {
    /* Make sure that no pending write process */
    case CTRL_SYNC :
      res = RES_OK;
      break;

    case CTRL_TRIM:
      nFrom = *((DWORD*)buff);
      nTo = *(((DWORD*)buff)+1);
      for(i = nFrom;i <= nTo;i ++){
      }
      res = RES_OK;
      break;
    
    /* Get number of sectors on the disk (DWORD) */
    case GET_SECTOR_COUNT :
//      *(DWORD*)buff = SECTOR_COUNT;  
      *(DWORD*)buff = (fatfs_partition->partition_length)/SECTOR_SIZE;
      res = RES_OK;
      break;
    
    /* Get R/W sector size (WORD) */
    case GET_SECTOR_SIZE :
      *(WORD*)buff = SECTOR_SIZE;   
      res = RES_OK;
      break;
    
    /* Get erase block size in unit of sector (DWORD) */
    case GET_BLOCK_SIZE :
      *(DWORD*)buff = BLOCK_SIZE; 
      res = RES_OK;
      break;
    
    default:
      res = RES_PARERR;
    }
  
  return res;
}
#endif /* _USE_IOCTL == 1 */

void MicoFlashEraseWrite( mico_partition_t partition, volatile uint32_t* off_set, uint8_t* data_addr ,uint32_t size )
{
  uint32_t f_sector;
  uint32_t f_addr;
  uint8_t *f_sector_buf = NULL;
  uint32_t pos = 0;
  
  uint16_t s_sector;
  
  f_sector = (*off_set) >> 12;
  f_addr = f_sector << 12; 
  
  s_sector = (*off_set) & 0x0F00; 
  
  f_sector_buf = malloc(FLASH_SECTOR); 
  
  MicoFlashRead( partition, &f_addr, f_sector_buf, FLASH_SECTOR); 
  
  for(pos=0; pos<size; pos++)
  {
    if( f_sector_buf[s_sector + pos] != 0xFF )
      break; 
  }
  
  if(pos != size){ 
    f_addr -= FLASH_SECTOR;
    MicoFlashErase(partition, f_addr, size); 
    
    for( pos=0; pos<size; pos++ )
    {
      f_sector_buf[s_sector + pos] = data_addr[pos];
    } 
    MicoFlashWrite( partition, &f_addr, f_sector_buf, FLASH_SECTOR);
  } else { 
    MicoFlashWrite( partition, off_set, data_addr, size ); 
  } 
  
  free(f_sector_buf);   
}
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

