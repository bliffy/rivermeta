#ifndef _WSDT_FIXEDSTRING_H
#define _WSDT_FIXEDSTRING_H

#include <ctype.h>
#include <stdint.h>
#include <string.h>

#define WSDT_FIXEDSTRING_STR "FIXEDSTRING_TYPE"

#define WSDT_FIXEDSTRING_LEN 1024

typedef struct _wsdt_fixedstring_t {
     int len;
     char buf[WSDT_FIXEDSTRING_LEN];
} wsdt_fixedstring_t;

typedef wsdt_fixedstring_t wsdt_fstr_t;

static inline int fixedstring_copy(
          wsdt_fstr_t * fstr,
          const char * buf,
          int len)
{
     if (len == 0) {
          fstr->len = 0;
          return 0;
     }
     if (len > WSDT_FIXEDSTRING_LEN) {
          len = WSDT_FIXEDSTRING_LEN;
     }
     fstr->len = len;
     memcpy((void * __restrict__)fstr->buf,
            (void * __restrict__)buf,
            len);
     return len;
}

static inline int fixedstring_copy_alnum(
          wsdt_fstr_t * fstr,
          const char * buf,
          int len)
{
     if (len == 0) {
          fstr->len = 0;
          return 0;
     }
     int outlen = 0;
     int last_space = 1;

     for (int i = 0; i < len; i++) {
          if (isalnum(buf[i]) || (buf[i] == '.')) {
               last_space = 0;
               fstr->buf[outlen] = buf[i];
               outlen++;
          }
          else if (!last_space) {
               last_space = 1;
               fstr->buf[outlen] = ' ';
               outlen++;
          }
          if (outlen >= WSDT_FIXEDSTRING_LEN) {
               break;
          }
     }
     
     fstr->len = outlen;
     return outlen;
}

#endif

