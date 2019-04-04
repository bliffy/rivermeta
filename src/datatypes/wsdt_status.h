#ifndef _WSDT_STATUS_H
#define _WSDT_STATUS_H

#include <time.h>

#define WSDT_STATUS_STR "STATUS_TYPE"

typedef struct _wsdt_status_t {
     time_t sec;
     int buflen;
     char buf[500];
} wsdt_status_t;

#endif

