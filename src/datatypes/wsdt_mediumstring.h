#ifndef _WSDT_MEDIUMSTRING_H
#define _WSDT_MEDIUMSTRING_H

#define WSDT_MEDIUMSTRING_STR "MEDIUMSTRING_TYPE"

#define WSDT_MEDIUMSTRING_LEN 16376

typedef struct _wsdt_mediumstring_t {
     int len;
     char buf[WSDT_MEDIUMSTRING_LEN];
} wsdt_mediumstring_t;

typedef wsdt_mediumstring_t wsdt_mstr_t;

static inline int mediumstring_copy(
          wsdt_mstr_t * mstr,
          void * buf,
          int len)
{
     if (len == 0) {
          mstr->len = 0;
          return 0;
     }
     if (mstr->len >= WSDT_MEDIUMSTRING_LEN) {
          return 0;
     }
     if (WSDT_MEDIUMSTRING_LEN < (mstr->len + len)) {
          len = WSDT_MEDIUMSTRING_LEN - mstr->len;
     }
     memcpy((void * __restrict__)(mstr->buf + mstr->len),
            (void * __restrict__)buf,
            len);
     mstr->len += len;
     return len;
}

#endif
