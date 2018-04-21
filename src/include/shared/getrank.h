#ifndef _GETRANK_H
#define _GETRANK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __FreeBSD__
#include <pthread_np.h>
// unfortunately, cpu_set_t is not defined in FreeBSD
typedef cpuset_t cpu_set_t;
#endif // __FreeBSD__

#ifdef WS_PTHREADS
extern __thread int thread_rank;
#define GETRANK() (thread_rank)
#else
#define GETRANK() (0)
#endif

#ifdef __cplusplus
}
#endif

#endif
