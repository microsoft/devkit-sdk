
#ifndef _json_inttypes_h_
#define _json_inttypes_h_

#include "json_config.h"

#ifdef JSON_C_HAVE_INTTYPES_H
/* inttypes.h includes stdint.h */
#include <inttypes.h>

#else
#include <stdint.h>

#define PRId64 "d"
#define SCNd64 "d"

#endif

#endif
