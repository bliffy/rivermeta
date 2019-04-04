#ifndef _WSDT_MMAP_H
#define _WSDT_MMAP_H

#include <stdint.h>

#define WSDT_MMAP_STR "MMAP_TYPE"

#define WSDT_MMAP_MAX_FILENAME_LEN 256

typedef struct _wsdt_mmap_t {
     int len;
     char * buf;
     int srcfd;
     char filename[WSDT_MMAP_MAX_FILENAME_LEN+1];
} wsdt_mmap_t;

#endif
