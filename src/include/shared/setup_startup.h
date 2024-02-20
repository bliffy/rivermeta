#ifndef _SETUP_STARTUP_H
#define _SETUP_STARTUP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "waterslide.h"
#include "sht_registry.h"
#include "shared/create_shared_vars.h"
#include "shared/lock_init.h"
#include "shared/getrank.h"

#ifdef __cplusplus
extern "C" {
#endif

// Macros for noop serial functions
#ifndef WS_PTHREADS
#define SETUP_STARTUP() 1
#else
#define SETUP_STARTUP() setup_startup()


static inline int setup_startup(void)
{
/* this does nothing
     cpu_set_t cpu_set;

     // Obtain list of available cpus on the processing
     // platform. Use this information to map threads to
     // appropriate cpu_ids.
     CPU_ZERO(&cpu_set);

     if(pthread_getaffinity_np(
             pthread_self(),
             sizeof(cpu_set),
             &cpu_set))
     {
          error_print("pthread_getaffinity_np failed");
          return 0;
     }
*/

     // init locks
     if (!lock_init())
          return 0;

     // init shared variables
     if (!create_shared_vars())
          return 0;

     return 1;
}

#endif // WS_PTHREADS

#ifdef __cplusplus
}
#endif

#endif

