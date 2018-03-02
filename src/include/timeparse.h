#ifndef _TIMEPARSE_H
#define _TIMEPARSE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dprint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STR2TIME_DEFAULT_FORMAT "%Y%m%d %H:%M:%S"
#define STR2TIME_WSSTD_FORMAT "%Y.%m.%d %H:%M:%S"
#if !defined(__USE_XOPEN) && !defined(__FreeBSD__)
#define _TIMEPARSE_UNUSABLE
#endif

#ifdef _TIMEPARSE_UNUSABLE
static inline time_t timeparse_str2time(
          const char * timestr,
          const char * format)
{ return 0; }
#else
static inline time_t timeparse_str2time(
          const char * timestr,
          const char * format)
{
     if (format == NULL)
          format = STR2TIME_DEFAULT_FORMAT;
     struct tm tm;
     if (strptime(timestr, format, &tm) == NULL)
          perror("strptime");
     return timegm(&tm);
}
#endif

#define MAX_FTIME 100
static inline time_t timeparse_file_time(
          const char *filename)
{
     char buffer[MAX_FTIME];
     const char * basename = strrchr(filename, '/');
     if ( basename == NULL )
          basename = filename;
     else
          basename++;
     const char * prefix_end = strrchr(basename, '.');
     if ( prefix_end == NULL )
          return 0;

     // search until number is found after period
     while (!isdigit(basename[0])) {
          const char * newbase = strchr(basename,'.');
          if ( newbase == NULL )
               break;
          basename = newbase + 1;
     }

     const int plen = prefix_end - basename;
     if (MAX_FTIME < plen )
          return 0;
     // the __restrict__ here tells the compiler that
     // these two pointers do not address overlapping
     // regions
     memcpy((void * __restrict__)buffer,
            (void * __restrict__)basename,
            plen);
     buffer[plen]   = '0';
     buffer[plen+1] = '0';
     buffer[plen+2] = '\0';
     return timeparse_str2time(buffer,"%Y%m%d.%H%M%S");
}

#define MAX_FTIME 100
static inline time_t timeparse_file_time2(
          const char *filename)
{
     char buffer[MAX_FTIME+4];
     const char * basename = strrchr(filename, '/');
     if ( basename == NULL )
          basename = filename;
     else
          basename++;
     const char * prefix_end = strrchr(basename, '.');
     if ( prefix_end == NULL )
          return 0;

     // search until number is found after period
     while (!isdigit(basename[0])) {
          const char * newbase = strchr(basename,'.');
          if ( newbase == NULL )
               break;
          basename = newbase + 1;
     }

     const int plen = prefix_end - basename;
     if (MAX_FTIME < plen ) {
          return 0;
     }
     memcpy(buffer, basename, plen);
     buffer[plen]   = '0';
     buffer[plen+1] = '0';
     buffer[plen+2] = '\0';
     return timeparse_str2time(buffer,"%Y%m%d%H%M%S");
}

#ifdef _TIMEPARSE_UNUSABLE
static inline int timeparse_detect_time(
          const char * buf,
          int len)
{ return 0; }
static inline int timeparse_detect_date(
          char * buf,
          int len)
{ return 0; }

#else
//return 1 if time detected but no usec time..
//return 2 if time has usec
//return 0 if time was not detected
static inline int timeparse_detect_time(
          const char * buf,
          int len)
{
     if ( (len < 8) || (len > 15) ) {
          dprint("invalid time - incorrect length");
          return 0;
     }
     //check periods
     if ( (buf[2] != ':') || (buf[5] != ':') ) {
          dprint("invalid time - nocolon");
          return 0;
     }
     if (len > 8) {
          if (buf[8] != '.')
               return 0;
          for (int i = 0; i < len; i++) {
               switch ( i ) {
               case 2: case 5: case 8: break;
               default:
                    if ( isdigit(buf[i]) ) break;
                    dprint("invalid time - non digit");
                    return 0;
               }
          }
          return 2;
     }
     else {
          for (int i = 0; i < 8; i++) {
               switch ( i ) {
               case 2: case 5: break;
               default:
                    if ( isdigit(buf[i]) ) break;
                    dprint("invalid time - non digit");
                    return 0;
               }
          }
          return 1;
     }
}

//return 1 if only date was detected
//return 2 if date and time was detected
//return 0 if date was not detected
static inline int timeparse_detect_date(
          const char * buf,
          int len)
{
     if ( (len < 10) || (len > 26) ) {
          dprint("invalid date - incorrect length");
          return 0;
     }
     int rtn = 1;
     if (len > 10) {
          rtn = 2;
          if ((buf[10] != ' ') || !timeparse_detect_time(buf + 11, len - 11)) {
               return 0;
          }
     }

     //check periods
     if ((buf[4] != '.') || (buf[7] != '.')) {
          dprint("invalid date - no periods");
          return 0;
     }
     for (int i = 0; i < 10; i++) {
          if (!isdigit(buf[i]) && (i != 4) && (i != 7)) {
               dprint("invalid date - not digit");
               return 0;
          }
     }
     return rtn;
}
#endif

#ifdef __cplusplus
}
#endif

#endif

