// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#include "SFlashBlockDevice.h"

#define BLOCK_SIZE                8
#define SECTOR_SIZE               512
//#define SECTOR_COUNT              2048
#define FLASH_SECTOR              4096

SFlashBlockDevice::SFlashBlockDevice(){
}

SFlashBlockDevice::~SFlashBlockDevice(){
}

int SFlashBlockDevice::init()
{
  fatfs_partition = MicoFlashGetInfo((mico_partition_t)MICO_PARTITION_FILESYS);
  return RES_OK;
}

int SFlashBlockDevice::deinit()
{
    return BD_ERROR_OK;
}

bd_size_t SFlashBlockDevice::get_read_size() const
{
    return SECTOR_SIZE;
}

bd_size_t SFlashBlockDevice::get_program_size() const
{
    return SECTOR_SIZE;
}

bd_size_t SFlashBlockDevice::get_erase_size() const
{
    return SECTOR_SIZE;
}

bd_size_t SFlashBlockDevice::size() const
{
    return fatfs_partition->partition_length;
}

int SFlashBlockDevice::read(void *b, bd_addr_t addr, bd_size_t size)
{
    DWORD sector = addr / SECTOR_SIZE;
    BYTE count = size / SECTOR_SIZE;
    SFLASHDISK_read((BYTE *)b, sector, count);
    return 0;
}

int SFlashBlockDevice::program(const void *b, bd_addr_t addr, bd_size_t size)
{
    DWORD sector = addr / SECTOR_SIZE;
    BYTE count = size / SECTOR_SIZE;
    SFLASHDISK_write((BYTE *)b, sector, count);
    return 0;
}

int SFlashBlockDevice::erase(bd_addr_t addr, bd_size_t size)
{
    MBED_ASSERT(is_valid_erase(addr, size));
    // TODO assert on programming unerased blocks

    return 0;
}


/**
  * @brief  Reads Sector(s)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT SFlashBlockDevice::SFLASHDISK_read(BYTE *buff, DWORD sector, BYTE count)
{
  DRESULT res = RES_OK;
  uint32_t offset;

  for(; count>0; count--)
  {
    offset = (uint32_t)sector*SECTOR_SIZE;
    MicoFlashRead((mico_partition_t)MICO_PARTITION_FILESYS, &offset, (uint8_t *)buff, SECTOR_SIZE);
    sector++;
    buff += SECTOR_SIZE;
  }
  return res;
}

void SFlashBlockDevice::MicoFlashEraseWrite( mico_partition_t partition, volatile uint32_t* off_set, uint8_t* data_addr ,uint32_t size )
{
  uint32_t f_sector;
  uint32_t f_addr;
  uint8_t *f_sector_buf = NULL;
  uint32_t pos = 0;
  
  uint16_t s_sector;
  
  f_sector = (*off_set) >> 12;
  f_addr = f_sector << 12; 
  
  s_sector = (*off_set) & 0x0F00; 
  
  f_sector_buf = (uint8_t*)malloc(FLASH_SECTOR); 
  
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

/**
  * @brief  Writes Sector(s)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT SFlashBlockDevice::SFLASHDISK_write(const BYTE *buff, DWORD sector, BYTE count)
{ 
  DRESULT res = RES_OK;
  uint32_t offset;

  for(; count>0; count--)
  {
    offset = (uint32_t)sector*SECTOR_SIZE;
    MicoFlashEraseWrite((mico_partition_t)MICO_PARTITION_FILESYS, &offset, (uint8_t *)buff, SECTOR_SIZE);
    sector++;
    buff += SECTOR_SIZE;
  }
  
  return res;
}
#endif