#ifndef _FILEOUT_H
#define _FILEOUT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "waterslide.h"
#include "waterslidedata.h"
#include "sysutil.h"
#include "datatypes/wsdt_tuple.h"
#include "datatypes/wsdt_binary.h"
#include "setup_exit.h"
#include <sys/time.h>
#include "stringhash5.h"
#include "zutil.h"

#define MAX_NAME_LEN 4000
#define MAX_FILE_VERSIONS 10000

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _fpdata_t {
     void* fp;
     time_t ts;
     char* filename;
     uint64_t recordcount;
     uint64_t bytecount;
     int rollovercount;
     int version;
     char* expandedname;
     char mode;
} fpdata_t;

typedef struct _filespec_t {
     int        dynamicfile;
     fpdata_t*  outfpdata;
     void*      outfp;  // might be FILE* or gzfile
     int        use_gzip;
     char       mode;
     char*      namepiece[7];
     int        namepiece_count;
     char*      labelname; 
     wslabel_t* label;
     stringhash5_t* fp_table;
     stringhash5_t* evicted_table;
     int      evicted_flag;
     char*    timespec;
     int      timeslice;
     uint64_t recordmax;
     uint64_t bytemax;
     char*    moveprefix;
     char*    fileprefix;
     char*    extension;
     int      safename;
} filespec_t;
  
void* fileout_parse_filespec(char* fspec, filespec_t* fs, int only_env);
fpdata_t* fileout_initialize(filespec_t* fs, void* type_table);
fpdata_t* fileout_select_file(wsdata_t* input, filespec_t* fs, time_t tm);
void fileout_filespec_cleanup(filespec_t* fs);

#ifdef __cplusplus
}
#endif

#endif // _FILEOUT_H
