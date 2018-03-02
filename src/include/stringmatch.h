#ifndef _STRINGMATCH_H
#define _STRINGMATCH_H

#include <stdint.h>
#include "error_print.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BMH_CHARSET_SIZE 256

typedef struct _stringmatch_t {
     int bmshift[BMH_CHARSET_SIZE];
     char * str;  //string to match
     char * str2; //.. other case.. used for nocase
     size_t len;
     size_t nocase;
} stringmatch_t;
     

stringmatch_t * stringmatch_init(
     const char *,
     size_t);
const char * stringmatch(
     const stringmatch_t *,
     const char *,
     size_t);
size_t stringmatch_offset(
     const stringmatch_t *,
     const char *,
     size_t);

stringmatch_t * stringmatch_init_nocase(
     const char *,
     size_t);
const char * stringmatch_nocase(
     const stringmatch_t *,
     const char *,
     size_t);
size_t stringmatch_offset_nocase(
     const stringmatch_t *,
     const char *,
     size_t);

void stringmatch_free(stringmatch_t *);

#ifdef __cplusplus
}
#endif

#endif // _STRINGMATCH_H
