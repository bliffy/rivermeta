#ifndef _ZUTIL_H
#define _ZUTIL_H

#include <stdint.h>
#include <zlib.h>
#include "sysutil.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline gzFile sysutil_open_timedfile_gz(
          const char * basefile,
          const char * extension,
          time_t filetime,
          time_t increment,
          char * outfilename,
          int outfilename_len)
{
     char workingfile[2000];
     gzFile rfp;

     if (sysutil_name_timedfile(basefile, extension, filetime, increment, workingfile, 2000) == 0) {
          return NULL;
     }

     //open file
     if ((rfp = gzopen(workingfile, "wb9")) == NULL) {
          perror("opening gzipped stats file for writing");
          error_print("could not open gzip output stats file %s",workingfile);
          return NULL;
     }

     if (outfilename) {
          int len = strlen(workingfile);
          //truncate somehow..
          if (len > outfilename_len) {
               len = outfilename_len;
          }
          memcpy(outfilename, workingfile, len);
          outfilename[len] = 0;
     }

     return rfp;
}

#ifdef __cplusplus
}
#endif

#endif // _ZUTIL_H

