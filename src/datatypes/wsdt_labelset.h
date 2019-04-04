#ifndef _WSDT_LABELSET_H
#define _WSDT_LABELSET_H

#include <stdint.h>

#include "waterslide.h"

#define WSDT_LABELSET_STR "LABELSET_TYPE"

#define WSDT_LABELSET_MAX 128

typedef struct _wsdt_labelset_t {
     int len;
     wslabel_t * labels[WSDT_LABELSET_MAX];
     uint64_t hash;
} wsdt_labelset_t;


static inline int wsdt_labelset_add(
          wsdt_labelset_t * ls,
          wslabel_t * label)
{
     if (ls->len >= WSDT_LABELSET_MAX) {
          return 0;
     }
     int i;
     for (i = 0; i < ls->len; i ++) {
          if (ls->labels[i] == label) {
               return 1;
          }
     }

     ls->labels[ls->len] = label;
     ls->len++;
     return 2;
}

static inline int wsdt_labelset_add_wsdata(
          wsdata_t * mlset,
          wslabel_t * label)
{
     wsdt_labelset_t * ls = (wsdt_labelset_t*)mlset->data;
     return wsdt_labelset_add(ls, label);
}

#endif

