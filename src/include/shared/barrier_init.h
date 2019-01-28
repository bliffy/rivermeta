#ifndef _CREATE_SPIN_BARRIER_H
#define _CREATE_SPIN_BARRIER_H

#include <sys/types.h>
#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
#include "win_mman.h"
#else
#include <sys/mman.h>
#endif
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WS_PTHREADS
#define BARRIER_INIT(barrier,nprocs) 1
#define BARRIER_WAIT(barrier) 
#else
#include <pthread.h>
extern pthread_barrier_t* barrier1;
#define BARRIER_TYPE_T pthread_barrier_t
extern uint32_t work_size;

// Macros for noop serial functions
#define BARRIER_INIT(barrier,nprocs) \
        barrier_init(barrier,nprocs) 
#define BARRIER_WAIT(barrier) \
        barrier_wait(barrier) 

// Prototypes
static inline int barrier_init(BARRIER_TYPE_T**, uint32_t);
static inline void barrier_wait(void*);

// Note: barrier_init() will only be called once for pthreads
static inline int barrier_init(
          BARRIER_TYPE_T** barrier,
          uint32_t nprocs)
{
     // pthread_barrier_init returns zero if successful
     return (0 == pthread_barrier_init(
          *barrier,
           NULL,
           work_size));
}

static inline void barrier_wait(void* barrier) {
     pthread_barrier_wait(barrier);
}

#endif // WS_PTHREADS

#ifdef __cplusplus
}
#endif

#endif
