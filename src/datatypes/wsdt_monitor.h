#ifndef _WSDT_MONITOR_H
#define _WSDT_MONITOR_H

// The following is used for health and status monitoring
// of data

#include <stdint.h>

#include "waterslide.h"

#define WSDT_MONITOR_STR "MONITOR_TYPE"

typedef struct _wsdt_monitor_t {
     wsdata_t * tuple;
     uint32_t visit;   // number of procs visited
     WS_SPINLOCK_DECL(lock);
} wsdt_monitor_t;


static inline wsdata_t * wsdt_monitor_get_tuple(
          wsdata_t * ws)
{
     wsdt_monitor_t * mon = (wsdt_monitor_t*)ws->data;
     return mon->tuple;
}

//used for polling
static inline uint32_t wsdt_monitor_get_visits(
          wsdata_t * ws)
{
     wsdt_monitor_t * mon = (wsdt_monitor_t*)ws->data;

#ifdef USE_ATOMICS
     return __sync_fetch_and_add(&mon->visit, 0);
#else
     uint32_t rtn;
     WS_SPINLOCK_LOCK(&mon->lock);
     rtn = mon->visit;
     WS_SPINLOCK_UNLOCK(&mon->lock);
     return rtn;
#endif
}


// used when kid finishes with monitoring
static inline void wsdt_monitor_set_visit(
          wsdata_t * ws)
{
     wsdt_monitor_t * mon = (wsdt_monitor_t*)ws->data;
#ifdef USE_ATOMICS
     __sync_fetch_and_add(&mon->visit, 1);
#else
     WS_SPINLOCK_LOCK(&mon->lock);
     mon->visit++;
     WS_SPINLOCK_UNLOCK(&mon->lock);
#endif
}

#endif

