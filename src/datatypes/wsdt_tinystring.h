#ifndef _WSDT_TINYSTRING_H
#define _WSDT_TINYSTRING_H

#define WSDT_TINYSTRING_STR "TINYSTRING_TYPE"

#define WSDT_TINYSTRING_LEN 124

typedef struct _wsdt_tinystring_t {
     int len;
     char buf[WSDT_TINYSTRING_LEN];
} wsdt_tinystring_t;

static inline int tinystring_copy(
          wsdt_tinystring_t * mstr,
          void * buf,
          int len)
{
     if (len == 0) {
          mstr->len = 0;
          return 0;
     }
     if (mstr->len >= WSDT_TINYSTRING_LEN) {
          return 0;
     }
     if (WSDT_TINYSTRING_LEN < (mstr->len + len)) {
          len = WSDT_TINYSTRING_LEN - mstr->len;
     }
     memcpy((void * __restrict__)(mstr->buf + mstr->len),
            (void * __restrict__)buf,
            len);
     mstr->len += len;
     return len;
}

#endif

