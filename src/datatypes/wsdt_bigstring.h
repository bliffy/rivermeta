#ifndef _WSDT_BIGSTRING_H
#define _WSDT_BIGSTRING_H

#define WSDT_BIGSTRING_STR "BIGSTRING_TYPE"

#define WSDT_BIGSTRING_LEN 40960

typedef struct _wsdt_bigstring_t {
     int len;
     char buf[WSDT_BIGSTRING_LEN];
} wsdt_bigstring_t;

typedef wsdt_bigstring_t wsdt_bstr_t;

#endif
