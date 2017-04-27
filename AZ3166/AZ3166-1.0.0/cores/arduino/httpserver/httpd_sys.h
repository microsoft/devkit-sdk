#ifndef _HTTPD_SYS_H_
#define _HTTPD_SYS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "httpd_utility.h"

int htsys_getln_soc(int sd, char *data_p, int buflen);

#ifdef __cplusplus
}
#endif

#endif



