#ifndef _CREATE_SHARED_VARS_H
#define _CREATE_SHARED_VARS_H

#include <string.h>
#include "waterslide.h"
#include "wsqueue.h"
#include "mimo.h"
#include "shared/lock_init.h"

#ifdef __cplusplus
extern "C" {
#endif

// Macros for noop serial functions
#ifndef WS_PTHREADS
#define CREATE_SHARED_VARS() 1
#else
#define CREATE_SHARED_VARS() create_shared_vars()

// Globals

extern uint32_t work_size;

WS_MUTEX_EXTERN( startlock );
WS_MUTEX_EXTERN( endgame_lock );
WS_MUTEX_EXTERN( exit_lock );

extern mimo_work_order_t** arglist;
extern pthread_t** thread;

#include <pthread.h>
extern pthread_barrier_t* barrier1;


static inline int create_shared_vars(void)
{
     // arglist contains info needed to start threads
     arglist = (mimo_work_order_t**)calloc(
          work_size, sizeof(mimo_work_order_t*));
     if (!arglist) {
          error_print("failed create_shared_vars calloc of arglist");
          return 0;
     }

     thread = (pthread_t**)calloc(
          work_size, sizeof(pthread_t*));
     if (!thread) {
          error_print("failed create_shared_vars calloc of thread");
          return 0;
     }

     for (int i=0; i<work_size; i++) {
          // Populate individual thread data (function
          // parameter).
          arglist[i] = (mimo_work_order_t*)calloc(
               1, sizeof(mimo_work_order_t));
          if (!arglist[i]) {
               error_print("failed create_shared_vars calloc of arglist[i]");
               return 0;
          }

          thread[i] = (pthread_t*)malloc(sizeof(pthread_t));
          if (!thread[i]) {
               error_print("failed create_shared_vars malloc of thread[i]");
               return 0;
          }
     }

     // Allocate a "barrier" in memory which will be shared
     // between the threads
     barrier1 = (pthread_barrier_t*)calloc(
          1, sizeof(pthread_barrier_t));
     if (!barrier1) {
          error_print("Failed to allocate memory for barrier1.");
          return 0;
     }

     return 1;
}

static inline void free_shared_vars(void)
{
     // Free the thread arglists
     register int i;
     for (i=0; i<work_size; i++) {
          free(arglist[i]);
     }
     free(arglist);
     // Free the barrier
     free(barrier1);
     // Last thing:  free the threads
     for (i=0; i<work_size; i++) {
          free(thread[i]);
     }
     free(thread);
}
#endif // WS_PTHREADS

#ifdef __cplusplus
}
#endif

#endif
