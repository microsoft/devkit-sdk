/**
******************************************************************************
* @file    fatfs_exfuns.c 
* @author  Ding QQ
* @version V1.0.0
* @date    21-May-2015
* @brief   MiCO RTOS Fatfs extern function.
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

#include "mico.h"
#include "fatfs_exfuns.h"



#define exfuns_log(M, ...) custom_log("FatFs_Ex", M, ##__VA_ARGS__)
#define exfuns_log_trace() custom_log_trace("FatFs_Ex")


filesystem_info fatfs_get_info()
{
  FATFS *fs;
  uint8_t res;
  DWORD fre_clust=0, fre_sect=0, tot_sect=0;
  
  filesystem_info info;
  
  memset(&info, 0x0, sizeof(filesystem_info));
  
  res = f_getfree((const TCHAR*)'0', &fre_clust, &fs);
  if(res==0)
    {											   
      tot_sect = (fs->n_fatent - 2) * fs->csize;
      fre_sect = fre_clust * fs->csize;		  
    #if _MAX_SS!=512
          tot_sect*=fs->ssize/512;
          fre_sect*=fs->ssize/512;
    #endif	  
          if(tot_sect<20480)
          {
              /* Print free space in unit of KB (assuming 512 bytes/sector) */
            info.total_space = tot_sect>>1;
            info.free_space = fre_sect>>1;
            info.unit = 'K';
            //exfuns_log("total drive space:%d KB; free space:%d KB", tot_sect>>1, fre_sect>>1);
          }else
          {
              /* Print free space in unit of KB (assuming 512 bytes/sector) */
            info.total_space = tot_sect>>11;
            info.free_space = fre_sect>>11;
            info.unit = 'M';
              //exfuns_log("total drive space:%d MB; free space :%d MB", tot_sect>>11, fre_sect>>11);
          }
    }
    return info;
}