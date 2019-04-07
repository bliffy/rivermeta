#ifndef __TIME_FUNCS_FIX_H__
#define __TIME_FUNCS_FIX_H__

#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
struct tm* win_gmtime_r(const time_t * src, struct tm* dest) {
     errno_t e = gmtime_s(dest, src);
     return ( e==0 ? dest : NULL );
}
#define GMTIME_R(s,d) win_gmtime_r(s,d)
#else
#define GMTIME_R(s,d) gmtime_r(s,d)
#endif

#ifdef __cplusplus
}
#endif

#endif
