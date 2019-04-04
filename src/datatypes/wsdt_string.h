#ifndef _WSDT_STRING_H
#define _WSDT_STRING_H

#include <stdint.h>

#define WSDT_STRING_STR "STRING_TYPE"

typedef struct _wsdt_string_t {
     int len;
     char * buf;
} wsdt_string_t;

typedef wsdt_string_t wsdt_str_t;

#endif

