#ifndef _WSDT_FLUSH_H
#define _WSDT_FLUSH_H

#include <stdio.h>
#include <stdint.h>

#define WSDT_FLUSH_EXIT_MSG 0x1337
#define WSDT_FLUSH_INLINE_MSG 0

#define WSDT_FLUSH_STR "FLUSH_TYPE"

typedef struct _wsdt_flush_t {
     uint32_t flag;
     char custom_msg[BUFSIZ];
} wsdt_flush_t;

#endif
