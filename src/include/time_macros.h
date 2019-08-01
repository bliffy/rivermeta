#ifndef __TIME_FUNCS_FIX_H__
#define __TIME_FUNCS_FIX_H__

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
#include <time.h>
#include <winsock2.h> // <- has its own timeval struct
#include "bsd_strptime.h"
#else
#include <time.h>
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)

static inline struct tm* win_gmtime_r(
          const time_t* src,
          struct tm* dest) {
     errno_t e = gmtime_s(dest, src);
     return ( e==0 ? dest : NULL );
}
#define GMTIME_R(s,d) win_gmtime_r(s,d)

static inline char* win_asctime_r(
          const struct tm* src,
          char* buf,
          size_t len) {
     return (0==asctime_s(buf, len, src) ? buf : NULL);
}
#define ASCTIME_R(s,b,z) win_asctime_r(s,b,z)

typedef long suseconds_t;


// taken from BSD sys/time
static inline void timeradd(
          struct timeval * a,
          struct timeval * b,
          struct timeval * result)
{
     (result)->tv_sec  = (a)->tv_sec  + (b)->tv_sec;
     (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;
     if ((result)->tv_usec >= 1000000)
     {
          (result)->tv_sec++;
          (result)->tv_usec -= 1000000;
     }
}
static inline void timersub(
          struct timeval * a,
          struct timeval * b,
          struct timeval * result)
{
     (result)->tv_sec  = (a)->tv_sec  - (b)->tv_sec;
     (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;
     if ((result)->tv_usec < 0) {
          (result)->tv_sec--;
          (result)->tv_usec += 1000000;
     }
}
/*
static inline void timerclear(struct timeval * tvp) {
     (tvp)->tv_sec = (tvp)->tv_usec = 0;
}
static inline int timerisset(struct timeval * tvp) {
     return (0!=tvp->tv_sec || 0!=tvp->tv_usec);
}
*/

#else // posix -------------------


#define GMTIME_R(s,d) gmtime_r(s,d)

#define ASCTIME_R(s,b,z) asctime_r(s,b)

#endif // end OS check

#ifdef __cplusplus
}
#endif

#endif
