#ifndef _WSDT_MASSIVESTRING_H
#define _WSDT_MASSIVESTRING_H

#include <stdint.h>

//the following is not a limit, just a big size
#define WSDT_MASSIVESTRING_LEN 8388608

#define WSDT_MASSIVESTRING_STR "MASSIVESTRING_TYPE"

typedef struct _wsdt_massivestring_t {
     int len;
     char * buf;
} wsdt_massivestring_t;

wsdata_t * wsdt_massivestring_alloc(int len, char ** pbuf);

#endif
