#ifndef __TIME_FUNCS_FIX_H__
#define __TIME_FUNCS_FIX_H__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
struct tm* win_gmtime_r(const time_t * src, struct tm* dest) {
     errno_t e = gmtime_s(dest, src);
     return ( e==0 ? dest : NULL );
}
#define GMTIME_R win_gmtime_r
#else
#define GMTIME_R gmtime_r
#endif

#ifdef __cplusplus
}
#endif

#endif
