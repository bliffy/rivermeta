#ifndef _KIDSHARE_H
#define _KIDSHARE_H

#include "waterslide.h"
#include "mimo.h"
#include "listhash.h"
#include "sysutil.h"
#include "shared/getrank.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WS_MAX_KIDSHARE (100)
typedef struct _mimo_kidshare_t {
     int cnt;
     void* data;
} mimo_kidshare_t;


static inline void* ws_kidshare_get(
          void* v_type_table,
          const char* label)
{
     if (!label)
          return NULL;
     int len = strlen(label);
     if (!len)
          return NULL;
     mimo_datalists_t* mdl = (mimo_datalists_t*)v_type_table;
     mimo_kidshare_t* ks;
     ks = (mimo_kidshare_t*)listhash_find(
          mdl->kidshare_table, label, len);
     if (!ks)
          return NULL;
     ks->cnt++;
     return ks->data;
}

static inline int ws_kidshare_put(
          void* v_type_table,
          const char* label,
          void* data)
{
     if (!label)
          return 0;
     int len = strlen(label);
     if (!len)
          return 0;
     mimo_datalists_t* mdl = (mimo_datalists_t*)v_type_table;
     mimo_kidshare_t* ks;
     ks = (mimo_kidshare_t*)listhash_find_attach(
          mdl->kidshare_table, label, len);
     if (!ks)
          return 0;
     ks->cnt++;
     ks->data = data;
     return 1;
}

// returns
//   -1  on error
//   0-N when unsharing
// .. if kid receives a 0 -- it means it can control
// deletion of shared data since it is the last to use it..
static inline int ws_kidshare_unshare(
          void* v_type_table,
          const char* label)
{
     if (!label) {
          // NULL label: the table is not shared, do nothing
          // this condition is detected in the
          // stringhash*_destroy functions, so this return
          // should never happen here
          return -1;
     }
     int len = strlen(label);
     if (!len) {
          // empty label: something BAD has happened, so
          // act as if the table is not shared
          return -2;
     }
     mimo_datalists_t* mdl = (mimo_datalists_t*)v_type_table;
     mimo_kidshare_t* ks;
     ks = (mimo_kidshare_t*)listhash_find(
          mdl->kidshare_table,
          label,
          len);
     if (!ks) {
          // this label is unknown: something REALLY BAD
          // has happened!
          return -3;
     }
     // when ks->cnt hits 0, the table can be freed by
     // the caller
     return (--ks->cnt);
}
#ifdef __cplusplus
}
#endif

#endif
