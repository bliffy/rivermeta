#ifndef _WSDT_TS_H
#define _WSDT_TS_H

#include <stdio.h>
#include "time_macros.h"

#define WSDT_TS_MSEC(s,u) (((s) * 1000) + (((u) / 1000) % 1000))

static inline int wsdt_print_ts_sec(
          FILE * stream,
          time_t tsec)
{
     int s;
     struct tm tdata;
     struct tm *tp;
     time_t stime;

     s = tsec % 86400;
     stime = tsec - s;
     tp = GMTIME_R(&stime, &tdata);
     return fprintf(stream,"%04d.%02d.%02d %02d:%02d:%02d",
                    tp->tm_year+1900,
                    tp->tm_mon+1, tp->tm_mday,
                    s / 3600, (s % 3600) / 60,
                    s % 60);
}

static inline int wsdt_print_ts(
          FILE * stream,
          wsdt_ts_t * ts)
{
     int rtn = 0;
     rtn += wsdt_print_ts_sec(stream,ts->sec);
     if (ts->usec) {
          rtn += fprintf(stream, ".%06u", (unsigned int)ts->usec);
     }
     return rtn;
}

static inline int wsdt_snprint_ts_sec(
          char * buf,
          int len,
          time_t tsec)
{
     int s;
     struct tm tdata;
     struct tm *tp;
     time_t stime;

     s = tsec % 86400;
     stime = tsec - s;
     tp = GMTIME_R(&stime, &tdata);
     return snprintf(buf, len, "%04d.%02d.%02d %02d:%02d:%02d",
                    tp->tm_year+1900,
                    tp->tm_mon+1, tp->tm_mday,
                    s / 3600, (s % 3600) / 60,
                    s % 60);
}

static inline int wsdt_snprint_ts(
          char * buf,
          int len,
          wsdt_ts_t * ts)
{
     int rtn;
     rtn = wsdt_snprint_ts_sec(buf, len, ts->sec);
     if ((rtn > 0) && ts->usec && (rtn < len)) {
          int l;
          l = snprintf(buf + rtn, len - rtn, ".%06u", (unsigned int)ts->usec);
          if (l > 0) {
               rtn += l;
          }
     }

     return rtn;
}

#endif

