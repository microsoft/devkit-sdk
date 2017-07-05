// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 

#ifndef SFLASH_BLOCK_DEVICE_H
#define SFLASH_BLOCK_DEVICE_H

#include "BlockDevice.h"
#include "mbed.h"
#include "diskio.h"
#include "mico.h"

/* SFlash : supervisor flash */
class SFlashBlockDevice : public BlockDevice
{
public:

    /** Lifetime of the memory block device
     */
    SFlashBlockDevice();
    virtual ~SFlashBlockDevice();

    /** Initialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual int init();

    /** Deinitialize a block device
     *
     *  @return         0 on success or a negative error code on failure
     */
    virtual int deinit();

    /** Read blocks from a block device
     *
     *  @param buffer   Buffer to read blocks into
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes, must be a multiple of read block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int read(void *buffer, bd_addr_t addr, bd_size_t size);

    /** Program blocks to a block device
     *
     *  The blocks must have been erased prior to being programmed
     *
     *  @param buffer   Buffer of data to write to blocks
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes, must be a multiple of program block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int program(const void *buffer, bd_addr_t addr, bd_size_t size);

    /** Erase blocks on a block device
     *
     *  The state of an erased block is undefined until it has been programmed
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes, must be a multiple of erase block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int erase(bd_addr_t addr, bd_size_t size);

    /** Get the size of a readable block
     *
     *  @return         Size of a readable block in bytes
     */
    virtual bd_size_t get_read_size() const;

    /** Get the size of a programable block
     *
     *  @return         Size of a programable block in bytes
     */
    virtual bd_size_t get_program_size() const;

    /** Get the size of a eraseable block
     *
     *  @return         Size of a eraseable block in bytes
     */
    virtual bd_size_t get_erase_size() const;

    /** Get the total size of the underlying device
     *
     *  @return         Size of the underlying device in bytes
     */
    virtual bd_size_t size() const;

private:
/* Private variables ---------------------------------------------------------*/
/* Disk status */
    DRESULT SFLASHDISK_read (BYTE*, DWORD, BYTE);
    #if _USE_WRITE == 1
        DRESULT SFLASHDISK_write (const BYTE*, DWORD, BYTE);
    #endif /* _USE_WRITE == 1 */
    static void MicoFlashEraseWrite( mico_partition_t partition, volatile uint32_t* off_set, uint8_t* data_addr ,uint32_t size );
    mico_logic_partition_t *fatfs_partition;
};


#endif
