#ifndef _WSDT_BINARY_H
#define _WSDT_BINARY_H

#include <stdint.h>

#define WSDT_BINARY_STR "BINARY_TYPE"

typedef struct _wsdt_binary_t {
     int len;
     char * buf;
} wsdt_binary_t;

typedef wsdt_binary_t wsdt_bin_t;

#endif
