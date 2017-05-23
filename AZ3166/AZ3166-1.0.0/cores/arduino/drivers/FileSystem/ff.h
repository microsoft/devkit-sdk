/*---------------------------------------------------------------------------/
/  FatFs - FAT file system module include R0.11a    (C)ChaN, 2015
/----------------------------------------------------------------------------/
/ FatFs module is a free software that opened under license policy of
/ following conditions.
/
/ Copyright (C) 2015, ChaN, all right reserved.
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/---------------------------------------------------------------------------*/


#ifndef _FATFS
#define _FATFS	64180	/* Revision ID */

#ifdef __cplusplus
extern "C" {
#endif

#include "integer.h"	/* Basic integer types */
#include "ffconf.h"		/* FatFs configuration options */
#if _FATFS != _FFCONF
#error Wrong configuration file (ffconf.h).
#endif

/** @addtogroup MICO_Middleware_Interface
  * @{
  */

/** @defgroup MICO_File_System MiCO File Sytem Operations
  * @brief Provide management APIs for file system
  * @{
  */


/* Definitions of volume management */

#if _MULTI_PARTITION		/* Multiple partition configuration */
typedef struct {
	BYTE pd;	/* Physical drive number */
	BYTE pt;	/* Partition: 0:Auto detect, 1-4:Forced partition) */
} PARTITION;
extern PARTITION VolToPart[];	/* Volume - Partition resolution table */
#define LD2PD(vol) (VolToPart[vol].pd)	/* Get physical drive number */
#define LD2PT(vol) (VolToPart[vol].pt)	/* Get partition index */

#else							/* Single partition configuration */
#define LD2PD(vol) (BYTE)(vol)	/* Each logical drive is bound to the same physical drive number */
#define LD2PT(vol) 0			/* Find first valid partition or in SFD */

#endif



/* Type of path name strings on FatFs API */

#if _LFN_UNICODE			/* Unicode string */
#if !_USE_LFN
#error _LFN_UNICODE must be 0 at non-LFN cfg.
#endif
#ifndef _INC_TCHAR
typedef WCHAR TCHAR;
#define _T(x) L ## x
#define _TEXT(x) L ## x
#endif

#else						/* ANSI/OEM string */
#ifndef _INC_TCHAR
typedef char TCHAR;
#define _T(x) x
#define _TEXT(x) x
#endif

#endif



/* File system object structure (FATFS) */

typedef struct {
	BYTE	fs_type;		/* FAT sub-type (0:Not mounted) */
	BYTE	drv;			/* Physical drive number */
	BYTE	csize;			/* Sectors per cluster (1,2,4...128) */
	BYTE	n_fats;			/* Number of FAT copies (1 or 2) */
	BYTE	wflag;			/* win[] flag (b0:dirty) */
	BYTE	fsi_flag;		/* FSINFO flags (b7:disabled, b0:dirty) */
	WORD	id;				/* File system mount ID */
	WORD	n_rootdir;		/* Number of root directory entries (FAT12/16) */
#if _MAX_SS != _MIN_SS
	WORD	ssize;			/* Bytes per sector (512, 1024, 2048 or 4096) */
#endif
#if _FS_REENTRANT
	_SYNC_t	sobj;			/* Identifier of sync object */
#endif
#if !_FS_READONLY
	DWORD	last_clust;		/* Last allocated cluster */
	DWORD	free_clust;		/* Number of free clusters */
#endif
#if _FS_RPATH
	DWORD	cdir;			/* Current directory start cluster (0:root) */
#endif
	DWORD	n_fatent;		/* Number of FAT entries, = number of clusters + 2 */
	DWORD	fsize;			/* Sectors per FAT */
	DWORD	volbase;		/* Volume start sector */
	DWORD	fatbase;		/* FAT start sector */
	DWORD	dirbase;		/* Root directory start sector (FAT32:Cluster#) */
	DWORD	database;		/* Data start sector */
	DWORD	winsect;		/* Current sector appearing in the win[] */
	BYTE	win[_MAX_SS];	/* Disk access window for Directory, FAT (and file data at tiny cfg) */
} FATFS;



/* File object structure (FIL) */

typedef struct {
	FATFS*	fs;				/* Pointer to the related file system object (**do not change order**) */
	WORD	id;				/* Owner file system mount ID (**do not change order**) */
	BYTE	flag;			/* Status flags */
	BYTE	err;			/* Abort flag (error code) */
	DWORD	fptr;			/* File read/write pointer (Zeroed on file open) */
	DWORD	fsize;			/* File size */
	DWORD	sclust;			/* File start cluster (0:no cluster chain, always 0 when fsize is 0) */
	DWORD	clust;			/* Current cluster of fpter (not valid when fprt is 0) */
	DWORD	dsect;			/* Sector number appearing in buf[] (0:invalid) */
#if !_FS_READONLY
	DWORD	dir_sect;		/* Sector number containing the directory entry */
	BYTE*	dir_ptr;		/* Pointer to the directory entry in the win[] */
#endif
#if _USE_FASTSEEK
	DWORD*	cltbl;			/* Pointer to the cluster link map table (Nulled on file open) */
#endif
#if _FS_LOCK
	UINT	lockid;			/* File lock ID origin from 1 (index of file semaphore table Files[]) */
#endif
#if !_FS_TINY
	BYTE	buf[_MAX_SS];	/* File private data read/write window */
#endif
} FIL;



/* Directory object structure (FATFS_DIR) */

typedef struct {
	FATFS*	fs;				/* Pointer to the owner file system object (**do not change order**) */
	WORD	id;				/* Owner file system mount ID (**do not change order**) */
	WORD	index;			/* Current read/write index number */
	DWORD	sclust;			/* Table start cluster (0:Root dir) */
	DWORD	clust;			/* Current cluster */
	DWORD	sect;			/* Current sector */
	BYTE*	dir;			/* Pointer to the current SFN entry in the win[] */
	BYTE*	fn;				/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
#if _FS_LOCK
	UINT	lockid;			/* File lock ID (index of file semaphore table Files[]) */
#endif
#if _USE_LFN
	WCHAR*	lfn;			/* Pointer to the LFN working buffer */
	WORD	lfn_idx;		/* Last matched LFN index number (0xFFFF:No LFN) */
#endif
#if _USE_FIND
	const TCHAR*	pat;	/* Pointer to the name matching pattern */
#endif
} FATFS_DIR;



/* File information structure (FILINFO) */

typedef struct {
	DWORD	fsize;			/* File size */
	WORD	fdate;			/* Last modified date */
	WORD	ftime;			/* Last modified time */
	BYTE	fattrib;		/* Attribute */
	TCHAR	fname[13];		/* Short file name (8.3 format) */
#if _USE_LFN
	TCHAR*	lfname;			/* Pointer to the LFN buffer */
	UINT 	lfsize;			/* Size of LFN buffer in TCHAR */
#endif
} FILINFO;



/* File function return code (FRESULT) */

typedef enum {
	FR_OK = 0,				/* (0) Succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive cannot work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any parameter error */
	FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_LOCK */
	FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
} FRESULT;



/*--------------------------------------------------------------*/
/* FatFs module application interface                           */
/*--------------------------------------------------------------*/

/** @brief   Open or create a file                 
  *
  * @param   fp:     Pointer to the blank file object
  * @param   path:   Pointer to the file name 
  * @param   mode:   Access mode and file open mode flags
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode);				



/** @brief   Close an open file object                  
  *
  * @param   fp:     Pointer to the file object
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_close (FIL* fp);		/* Close an open file object */



/** @brief   Read File                    
  *
  * @param   fp:     Pointer to the file object
  * @param   buff:   Pointer to data buffer
  * @param   btr:    Number of bytes to read
  * @param   br:     Pointer to number of bytes read
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);			


/** @brief   Write data to a File                    
  *
  * @param   fp:     Pointer to the file object
  * @param   buff:   Pointer to the data to be written 
  * @param   btw:    Number of bytes to write
  * @param   bw:     Pointer to number of bytes written
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw);	

/** @brief   Forward data to the stream directly (available on only tiny cfg)                    
  *
  * @param   fp:     Pointer to the file object
  * @param   UINT(*func)(const BYTE*,UINT):   Pointer to the streaming function 
  * @param   btf:    Number of bytes to forward 
  * @param   bf:     Pointer to number of bytes forwarded 
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	


/** @brief   Move file pointer of a file object                  
  *
  * @param   fp:     Pointer to the file object
  * @param   ofs:    File pointer from top of file
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_lseek (FIL* fp, DWORD ofs);								


/** @brief   Truncate file                  
  *
  * @param   fp:     Pointer to the file object
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_truncate (FIL* fp);										/* Truncate file */



/** @brief   Flush cached data of a writing file                 
  *
  * @param   fp:     Pointer to the file object
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_sync (FIL* fp);


/** @brief   Open a directory                 
  *
  * @param   dp:     Pointer to directory object to create 
  * @param   path:   Pointer to the directory path 
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_opendir (FATFS_DIR* dp, const TCHAR* path);						


/** @brief   Close an open directory                
  *
  * @param   dp:    Pointer to the directory object to be closed 
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_closedir (FATFS_DIR* dp);	


/** @brief   Read a directory item               
  *
  * @param   dp:    Pointer to the open directory object
  * @param  fno: Pointer to file information to return
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_readdir (FATFS_DIR* dp, FILINFO* fno);	


/** @brief   Find first file              
  *
  * @param   dp:    Pointer to the blank directory object
  * @param  fno:   Pointer to the file information structure 
  * @param  path: Pointer to the directory to open 
  * @param  pattern:  Pointer to the matching pattern
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_findfirst (FATFS_DIR* dp, FILINFO* fno, const TCHAR* path, const TCHAR* pattern);	


/** @brief   Find next file             
  *
  * @param   dp:    Pointer to the blank directory object
  * @param  fno:   Pointer to the file information structure 
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_findnext (FATFS_DIR* dp, FILINFO* fno);							


/** @brief   Create a sub directory           
  *
  * @param   path: Pointer to the directory path
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_mkdir (const TCHAR* path);								


/** @brief   Delete an existing file or directory         
  *
  * @param   path: Pointer to the file or directory path
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_unlink (const TCHAR* path);				


/** @brief   Rename/Move a file or directory         
  *
  * @param   path_old:   Pointer to the object to be renamed 
  * @param   path_new:   Pointer to the new name 
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new);	


/** @brief   Get file status         
  *
  * @param   path:   Pointer to the file path 
  * @param   fno:    Pointer to file information to return
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_stat (const TCHAR* path, FILINFO* fno);					


/** @brief   Change attribute of the file/dir       
  *
  * @param   path:   Pointer to the file path 
  * @param   attr:   Attribute bits
  * @param   mask:   Attribute mask to change
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_chmod (const TCHAR* path, BYTE attr, BYTE mask);			


/** @brief   Change times-tamp of the file/dir    
  *
  * @param   path:  Pointer to the file/directory name
  * @param   fno:   Pointer to the time stamp to be set
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_utime (const TCHAR* path, const FILINFO* fno);			


/** @brief   Change current directory    
  *
  * @param   path:  Pointer to the directory path
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_chdir (const TCHAR* path);								


/** @brief   Change current drive   
  *
  * @param   path:  Drive number
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_chdrive (const TCHAR* path);								


/** @brief   Get current directory   
  *
  * @param   buff:  Pointer to the directory path
  * @param   len:   size of path
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_getcwd (TCHAR* buff, UINT len);							


/** @brief   Get number of free clusters on the drive  
  *
  * @param   path:  Path name of the logical drive number
  * @param   nclst: Pointer to a variable to return number of free clusters
  * @param   fatfs: Pointer to return pointer to corresponding file system object
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);	


/** @brief   Get volume label 
  *
  * @param   path:  Path name of the logical drive number
  * @param   label: Pointer to a buffer to return the volume label
  * @param   vsn:   Pointer to a variable to return the volume serial number
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_getlabel (const TCHAR* path, TCHAR* label, DWORD* vsn);	


/** @brief   Set volume label  
  *
  * @param   label: Pointer to the volume label to set 
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_setlabel (const TCHAR* label);							


/** @brief   Mount/Unmount a logical drive 
  *
  * @param   fs:    Pointer to the file system object (NULL:unmount)
  * @param   path:  Logical drive number to be mounted/unmounted 
  * @param   opt:   0:Do not mount (delayed mount), 1:Mount immediately 
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt);


/** @brief   Create a file system on the volume
  *
  * @param   path: Logical drive number
  * @param   sfd:  Partitioning rule 0:FDISK, 1:SFD
  * @param   au:   Size of allocation unit in unit of byte or sector
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_mkfs (const TCHAR* path, BYTE sfd, UINT au);				



/** @brief   Divide a physical drive into some partitions
  *
  * @param   pdrv: Physical drive number 
  * @param   szt:  Pointer to the size table for each partitions 
  * @param   work: Pointer to the working buffer 
  *
  * @return  FR_OK(0): The object is valid, !=0: Invalid 
  */
FRESULT f_fdisk (BYTE pdrv, const DWORD szt[], void* work);


/** @brief   Put a character to the file 
  *
  * @param   c:   A character to be output 
  * @param   fp:  Pointer to the file object
  *
  * @return  ?
  */
int f_putc (TCHAR c, FIL* fp);		


/** @brief   Put a string to the file 
  *
  * @param   str:   Pointer to the string to be output
  * @param   cp:    Pointer to the file object
  *
  * @return  ?
  */
int f_puts (const TCHAR* str, FIL* cp);		


/** @brief   Put a formatted string to the file 
  *
  * @param   fp     Pointer to the file object 
  * @param   str:   Pointer to the format string
  * @param   ...:   Optional arguments...
  *
  * @return  ?
  */
int f_printf (FIL* fp, const TCHAR* str, ...);		


/** @brief   Get a string from the file 
  *
  * @param   buff:  Pointer to the string buffer to read 
  * @param   len:   Size of string buffer (characters) 
  * @param    fp:   Pointer to the file object
  *
  * @return  ?
  */
TCHAR* f_gets (TCHAR* buff, int len, FIL* fp);					

/**
  * @}
  */

/**
  * @}
  */


#define f_eof(fp) ((int)((fp)->fptr == (fp)->fsize))
#define f_error(fp) ((fp)->err)
#define f_tell(fp) ((fp)->fptr)
#define f_size(fp) ((fp)->fsize)
#define f_rewind(fp) f_lseek((fp), 0)
#define f_rewinddir(dp) f_readdir((dp), 0)

#ifndef EOF
#define EOF (-1)
#endif




/*--------------------------------------------------------------*/
/* Additional user defined functions                            */

/* RTC function */
#if !_FS_READONLY && !_FS_NORTC
DWORD get_fattime (void);
#endif

/* Unicode support functions */
#if _USE_LFN							/* Unicode - OEM code conversion */
WCHAR ff_convert (WCHAR chr, UINT dir);	/* OEM-Unicode bidirectional conversion */
WCHAR ff_wtoupper (WCHAR chr);			/* Unicode upper-case conversion */
#if _USE_LFN == 3						/* Memory functions */
void* ff_memalloc (UINT msize);			/* Allocate memory block */
void ff_memfree (void* mblock);			/* Free memory block */
#endif
#endif

/* Sync functions */
#if _FS_REENTRANT
int ff_cre_syncobj (BYTE vol, _SYNC_t* sobj);	/* Create a sync object */
int ff_req_grant (_SYNC_t sobj);				/* Lock sync object */
void ff_rel_grant (_SYNC_t sobj);				/* Unlock sync object */
int ff_del_syncobj (_SYNC_t sobj);				/* Delete a sync object */
#endif




/*--------------------------------------------------------------*/
/* Flags and offset address                                     */


/* File access control and file status flags (FIL.flag) */

#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00

#if !_FS_READONLY
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20
#define FA__DIRTY			0x40
#endif


/* FAT sub type (FATFS.fs_type) */

#define FS_FAT12	1
#define FS_FAT16	2
#define FS_FAT32	3


/* File attribute bits for directory entry */

#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_MASK	0x3F	/* Mask of defined bits */


/* Fast seek feature */
#define CREATE_LINKMAP	0xFFFFFFFF



/*--------------------------------*/
/* Multi-byte word access macros  */

#if _WORD_ACCESS == 1	/* Enable word access to the FAT structure */
#define	LD_WORD(ptr)		(WORD)(*(WORD*)(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(*(DWORD*)(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(WORD*)(BYTE*)(ptr)=(WORD)(val)
#define	ST_DWORD(ptr,val)	*(DWORD*)(BYTE*)(ptr)=(DWORD)(val)
#else					/* Use byte-by-byte access to the FAT structure */
#define	LD_WORD(ptr)		(WORD)(((WORD)*((BYTE*)(ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(((DWORD)*((BYTE*)(ptr)+3)<<24)|((DWORD)*((BYTE*)(ptr)+2)<<16)|((WORD)*((BYTE*)(ptr)+1)<<8)|*(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8)
#define	ST_DWORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8); *((BYTE*)(ptr)+2)=(BYTE)((DWORD)(val)>>16); *((BYTE*)(ptr)+3)=(BYTE)((DWORD)(val)>>24)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FATFS */
