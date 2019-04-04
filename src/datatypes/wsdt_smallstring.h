#ifndef _WSDT_SMALLSTRING_H
#define _WSDT_SMALLSTRING_H

#define WSDT_SMALLSTRING_STR "SMALLSTRING_TYPE"

#define WSDT_SMALLSTRING_LEN 1600

typedef struct _wsdt_smallstring_t {
     int len;
     char buf[WSDT_SMALLSTRING_LEN];
} wsdt_smallstring_t;

static inline int smallstring_copy(
          wsdt_smallstring_t * mstr,
          void * buf,
          int len)
{
     if (len == 0) {
          mstr->len = 0;
          return 0;
     }
     if (mstr->len >= WSDT_SMALLSTRING_LEN) {
          return 0;
     }
     if (WSDT_SMALLSTRING_LEN < (mstr->len + len)) {
          len = WSDT_SMALLSTRING_LEN - mstr->len;
     }
     memcpy((void * __restrict__)(mstr->buf + mstr->len),
            (void * __restrict__)buf,
            len);
     mstr->len += len;
     return len;
}

#endif
