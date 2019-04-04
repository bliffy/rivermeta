
#ifndef _WSDT_ARRAY_DOUBLE_H
#define _WSDT_ARRAY_DOUBLE_H

#include <stdint.h>
//#include "waterslide.h"

#define WSDT_ARRAY_DOUBLE_STR "ARRAY_DOUBLE_TYPE"

#define WSDT_ARRAY_DOUBLE_MAX 32

typedef struct _wsdt_array_double_t {
     int len;
     double value[WSDT_ARRAY_DOUBLE_MAX];
} wsdt_array_double_t;


static inline int wsdt_array_double_add(
          wsdt_array_double_t * au,
          double value)
{
     if (au->len >= WSDT_ARRAY_DOUBLE_MAX) {
          return 0;
     }
     au->value[au->len] = value;
     au->len++;
     return 1;
}

static inline int wsdt_array_double_add_wsdata(
          wsdata_t * auset,
          double value)
{
     wsdt_array_double_t * au = (wsdt_array_double_t*)auset->data;
     return wsdt_array_double_add(au, value);
}

#endif

