#ifndef _SYSUTIL_H
#define _SYSUTIL_H

#include <stdint.h>
#include <time.h>
//#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *    structure for determining when a time boundary is reached
 *       .. useful in creating files every N minutes
 */
typedef struct _time_boundary_t {
     time_t current_boundary; // current block timestamp
     time_t prev_boundary;    // prev timestamp block
     time_t boundary_ts;   // timestamp of next time boundary
     time_t increment_ts;  // increment for each boundary
} time_boundary_t;

time_t sysutil_get_duration_ts(const char * optarg);

static inline void sysutil_print_webchars(
          FILE * stream,
          size_t len,
          const char* str)
{
     size_t i;
     for (i = 0; i < len; i++) {
          switch(str[i]) {
          case '<':
               fprintf(stream, "&lt;");
               break;
          case '>':
               fprintf(stream, "&gt;");
               break;
          case '&':
               fprintf(stream, "&amp;");
               break;
          default:
               fprintf(stream, "%c", str[i]);
          }
     }
     fprintf(stream, "\n");
}

void sysutil_print_content_ascii(
     FILE *,
     const char* /*content*/,
     size_t /*clen*/);
void sysutil_print_content(
     FILE * ,
     const char* ,
     size_t );
void sysutil_print_content_web(
     FILE * ,
     const char* ,
     size_t );
void sysutil_print_content_hex(
     FILE * ,
     const char* ,
     size_t );
void sysutil_print_content_strings(
     FILE * ,
     const char* /*buf*/,
     size_t /*buflen*/,
     size_t /*runlen*/);
void sysutil_print_content_strings_web(
     FILE * ,
     const char* /*buf*/, 
     size_t /*buflen*/,
     size_t /*runlen*/);

void sysutil_rename_file(
     const char* /*source file*/,
     const char* /*destination file*/);

int sysutil_name_timedfile(
     const char* /*basefile*/,
     const char* /*extension*/,
     time_t /*filetime*/,
     time_t /*increment*/,
     char* /*outfilename*/,   //<- param out
     size_t /*outfilename_len*/);



FILE * sysutil_open_timedfile(
     const char* /*basefile*/,
     const char* /*extension*/,
     time_t /*filetime*/,
     time_t /*increment*/,
     char* /*outfilename*/,      //<- param out
     size_t /*outfilename_len*/);

uint64_t sysutil_get_strbytes(const char* optarg);
time_t sysutil_get_duration_ts(const char* optarg);

time_t sysutil_get_hourly_increment_ts(const char* optarg);
int sysutil_test_time_boundary(
     time_boundary_t* tb,
     time_t current_time);
void sysutil_print_time_interval(FILE * fp, time_t t);

void sysutil_printts(FILE * stream, time_t sec, time_t usec);
void sysutil_printts_sec(FILE * stream, time_t sec);
int sysutil_snprintts(
     char* buf,
     size_t len,
     time_t sec,
     time_t usec);
int sysutil_snprintts_sec(
     char* buf,
     size_t len,
     time_t sec);
int sysutil_snprintts_sec2(
     char* buf,
     size_t len,
     time_t sec);

//return 1 if file exists and has length > 0
int sysutil_file_exists(const char* /*filename*/);

FILE * sysutil_config_fopen(const char*, const char*);
void sysutil_config_fclose(FILE *);
int set_sysutil_pConfigPath(const char*);
int get_sysutil_pConfigPath(uint32_t, char**, unsigned int*);
void free_sysutil_pConfigPath(void);
int sysutil_prepend_config_path(char**);

int sysutil_decode_hex_escapes(
     char* /*str*/,
     size_t* /*len*/);

#ifdef __cplusplus
}
#endif

#endif // _SYSUTIL_H
