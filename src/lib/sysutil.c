//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "time_macros.h"
#include "sysutil.h"
#include "waterslide.h"
#include <limits.h>

#define SCRATCHPAD_LEN 1500

#define MAX_PATH_COUNT 31

static char * sysutil_pConfigPath = NULL;
static char * sysutil_pConfigPaths[MAX_PATH_COUNT+1] = {NULL};

void sysutil_print_content(
          FILE* stream,
          const char* content,
          size_t clen) {
     size_t i,k;
     char seg16[17];

     memset(seg16,0,17);
     k = 15;

     //print buf contents
     for (i = 0; i < clen; i++) {
          k = i % 16;

          if (k == 0)
               fprintf(stream, " ");

          fprintf(stream,"%02x ", (uint8_t)content[i]);
          if (isprint(content[i]))
               seg16[k] = content[i];
          else
               seg16[k] = '.';

          switch(k) {
          case 7:
               fprintf(stream, " ");
               break;
          case 15:
               fprintf(stream,"  %s\n", seg16);
               memset(seg16,0,17);
               break;
          }
     }

     if (k != 15) {
          for (i = k; i < 15; i++) {
               switch(i) {
               case 7:
                    if (k == 7)
                         fprintf(stream,"   ");
                    else
                         fprintf(stream,"    ");
                    break;
               case 15:
                    fprintf(stream,"  ");
                    break;
               default:
                    fprintf(stream,"   ");
               }
          }

          fprintf(stream,"  %s\n", seg16);
     }
}

#define MAX_WEB_LINE 100
void sysutil_print_content_web(
          FILE* stream,
          const char* content,
          size_t clen) {
     size_t i,k;
     char seg16[MAX_WEB_LINE];

     k = 15;
     size_t s = 0;

     //print buf contents
     for (i = 0; i < clen; i++) {
          k = i % 16;
          if (k == 0) {
               fprintf(stream, " ");
               s = 0;
          }
          fprintf(stream,"%02x ", (uint8_t)content[i]);
          if (isprint(content[i])) {
               switch(content[i]) {
               case '<':
                    snprintf(seg16 + s, MAX_WEB_LINE - s, "&lt;");
                    s += 4; 
                    break;
               case '>':
                    snprintf(seg16 + s, MAX_WEB_LINE - s, "&gt;");
                    s += 4; 
                    break;
               case '&':
                    snprintf(seg16 + s, MAX_WEB_LINE - s, "&amp;");
                    s += 5; 
                    break;
               default:
                    seg16[s] = content[i];
                    s++;
               }  
          }
          else {
               seg16[s] = '.';
               s++;
          }

          switch(k) {
          case 7:
               fprintf(stream, " ");
               break;
          case 15:
               fprintf(stream,"  %.*s\n", (unsigned int)s, seg16);
               break;
          }
     }

     if (k != 15) {
          for (i = k; i < 15; i++) {
               switch(i) {
               case 7:
                    if (k == 7) {
                         fprintf(stream,"   ");
                    }
                    else {
                         fprintf(stream,"    ");
                    }
                    break;
               case 15:
                    fprintf(stream,"  ");
                    break;
               default:
                    fprintf(stream,"   ");
               }
          }

          fprintf(stream,"  %.*s\n", (unsigned int)s, seg16);
     }
}


void sysutil_print_content_ascii(
          FILE* stream,
          const char* content,
          size_t clen)
{
     for (size_t i = 0; i < clen; i++) {
          if (isprint(content[i]))
               fprintf(stream, "%c", content[i]);
          else
               fprintf(stream, ".");
     }
}

void sysutil_print_content_hex(
          FILE* stream,
          const char* content,
          size_t clen) {
     for (size_t i = 0; i < clen; i++) {
          fprintf(stream, " %02x", (uint8_t)content[i]);
     }
}



void sysutil_print_content_strings(
          FILE* stream,
          const char* content,
          size_t clen,
          size_t runlen) {
     size_t crun = 0;
     size_t printlines = 0;
     const char* startp = NULL;
     for (size_t i = 0; i < clen; i++) {
          if (isprint(content[i]) || (content[i] == '\t')) {
               if (!crun)
                    startp = &content[i];
               crun++;
          }
          else {
               if (crun >= runlen) {
                    fprintf(stream, "%.*s\n", (unsigned int)crun, startp);
                    printlines++;
               }
               crun = 0;
               startp = NULL;
          }
     }
     if (crun && ((crun >= runlen) || !printlines)) {
          fprintf(stream, "%.*s\n", (unsigned int)crun, startp);
     }
}

void sysutil_print_content_strings_web(
          FILE* stream,
          const char* content,
          size_t clen,
          size_t runlen) {
     size_t crun = 0;
     size_t printlines = 0;
     const char* startp = NULL;
     for (size_t i = 0; i < clen; i++) {
          if (isprint(content[i]) || (content[i] == '\t')) {
               if (!crun)
                    startp = &content[i];
               crun++;
          }
          else {
               if (crun >= runlen) {
                    sysutil_print_webchars(
                         stream, crun, startp);
                    printlines++;
               }
               crun = 0;
               startp = NULL;
          }
     }
     if (crun && ((crun >= runlen) || !printlines)) {
          sysutil_print_webchars(stream, crun, startp);
     }
}

void sysutil_rename_file(
          const char* sourcefile,
          const char* destfile)
{
     if (rename(sourcefile, destfile) == -1) {
          perror("renaming file");
          error_print("could not rename file");
     }
}


#define SYSUTIL_TIME_STRING_MAX 20
#define SYSUTIL_SUFFIX_LEN 5
int sysutil_name_timedfile(
          const char* basefile,
          const char* extension,
          time_t filetime,
          time_t increment,
          char* outfilename,
          size_t outfilename_len)
{
     size_t prefixlen = 0;
     size_t midlen = 0;
     size_t extension_offset = 0;
     size_t extension_len = 0;
     size_t suffix_cnt = 0;
     struct tm tmsec;
     struct stat statbuffer;
     char workingfile[2000];
     size_t len;

     if (!basefile)
          return 0;
     if (extension)
          extension_len = strlen(extension);
     prefixlen = strlen(basefile);
     len = prefixlen + SYSUTIL_TIME_STRING_MAX + 
          extension_len + SYSUTIL_SUFFIX_LEN + 1;

     if (len > 2000) {
          error_print("unable to alloc timestamp file - filename too big");
          return 0;
     }

     strncpy(workingfile, basefile, prefixlen);

     //get name of file...
     GMTIME_R(&filetime, &tmsec);

     //if per hour or per minute or per second
     if (increment % 60) {
          midlen = strftime(
               workingfile + prefixlen,
               SYSUTIL_TIME_STRING_MAX,
               "%Y%m%d.%H%M.%S",
               &tmsec);
     }
     else if (increment % 3600) {
          midlen = strftime(
               workingfile + prefixlen,
               SYSUTIL_TIME_STRING_MAX,
               "%Y%m%d.%H%M",
               &tmsec);
     }
     else {
          midlen = strftime(
               workingfile + prefixlen,
               SYSUTIL_TIME_STRING_MAX,
               "%Y%m%d.%H",
               &tmsec);
     }
     extension_offset = midlen + prefixlen;

     snprintf(workingfile + extension_offset,
              len - extension_offset,
              "%s", extension);
     strncpy(workingfile + midlen + prefixlen,
             extension, extension_len + 1);

     suffix_cnt++;

     while (!stat(workingfile, &statbuffer)) {
          snprintf(workingfile + extension_offset,
                   len - extension_offset,
                   ".%03" PRIu64 "%s", suffix_cnt, extension);
          suffix_cnt++;
          if (suffix_cnt >= 10000) {
               error_print("max files reached");
               return 0;
          }
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

     return 1;
}

FILE * sysutil_open_timedfile(
          const char* basefile,
          const char* extension,
          time_t filetime,
          time_t increment,
          char* outfilename,
          size_t outfilename_len)
{
     char workingfile[2000];
     FILE * rfp;

     if (sysutil_name_timedfile(basefile, extension, filetime, increment, workingfile, 2000) == 0) {
          return NULL;
     }

     //open file
     if ((rfp = fopen(workingfile, "w")) == NULL) {
          perror("opening stats file for writing");
          error_print("could not open output stats file");
          return NULL;
     }

     if (outfilename) {
          size_t len = strlen(workingfile);
          if (len > outfilename_len) {
               len = outfilename_len;
          }
          memcpy(outfilename, workingfile, len);
          outfilename[len] = 0;
     }

     return rfp;
}



// read in string and return number based on string
uint64_t sysutil_get_strbytes(const char* optarg) {
     if (strchr(optarg,'k') || strchr(optarg,'K')) {
          return (uint64_t)strtoull(optarg, NULL, 10) * 1024;
     }
     else if (strchr(optarg,'m') || strchr(optarg,'M')) {
          return (uint64_t)strtoull(optarg, NULL, 10) * 1024 * 1024;
     }
     else if (strchr(optarg,'g') || strchr(optarg,'G')) {
          return (uint64_t)strtoull(optarg, NULL, 10) * 1024 * 1024 * 1024;
     }
     else {
          return (uint64_t)strtoull(optarg, NULL, 10);
     }
}

time_t sysutil_get_duration_ts(const char * optarg) {
     time_t increment_ts = 0;

     if (strchr(optarg,'h') || strchr(optarg,'H')) {
      increment_ts = atoi(optarg) * 60 * 60;
     }
     else if (strchr(optarg,'m') || strchr(optarg,'M')) {
      increment_ts = atoi(optarg) * 60;
     }
     else if (strchr(optarg,'s') || strchr(optarg,'S')) {
      increment_ts = atoi(optarg);
     }
     else {
      fprintf(stderr, "time unit not specified or invalid...");
      fprintf(stderr, "defaulting to seconds\n");
      increment_ts = atoi(optarg);
     }
     if(increment_ts <= 0) {
      fprintf(stderr, "invalid time specified: %s\n", optarg);
      increment_ts = 0;
     }
     return increment_ts;
}
//read string and return time interval
//check to see if string is divisible by hour
//returns 0 on error
time_t sysutil_get_hourly_increment_ts(const char* optarg) {
     time_t increment_ts = sysutil_get_duration_ts(optarg);
     if ( (increment_ts >= 3600) && ((increment_ts % 3600) == 0) ) {
      return increment_ts;
     }
     else {
      fprintf(stderr, "invalid time specified, not a multiple of 1 hour: %"PRIu64" will be changed to 0\n", (uint64_t)increment_ts);
     }
     return 0;
}

//set next time boundary. (used by sysutil_test_time_boundary)
static void sysutil_next_time_boundary(
          time_boundary_t* tb,
          time_t current_time)
{
     if (!tb)
          return;

     //align boundary to bottom of the hour
     tb->boundary_ts = current_time - (current_time % 3600);

     tb->prev_boundary = tb->current_boundary;

     //align to increment minute
     do {
          tb->current_boundary = tb->boundary_ts;
          tb->boundary_ts += tb->increment_ts;
     }
     while (tb->boundary_ts <= current_time);
}

/* see if current time is at a time boundary return 1 or 2 if true
 * returns 2 if this is the first time boundary was tested..
 * returns 1 if time boundary was reached
 * returns 0 if time boundary was not reached
 */
int sysutil_test_time_boundary(
          time_boundary_t* tb,
          time_t current_time)
{
     if ((tb == NULL) || (tb->increment_ts == 0))
          return 0;
     else if (tb->boundary_ts == 0) {
          //this is the first time we checked this info
          sysutil_next_time_boundary(tb, current_time);
          return 2;
     }
     else if (current_time >= tb->boundary_ts) {
          sysutil_next_time_boundary(tb, current_time);
          return 1;
     }
     return 0;
}

void sysutil_print_time_interval(FILE * fp, time_t t) {
     time_t i;
     if (t == 0)
          fprintf(fp, "0 seconds");
     else if ((t % 3600) == 0) {
          i = t / 3600;
          if ((int)i == 1)
               fprintf(fp,"1 hour");
          else
               fprintf(fp,"%" PRIu64 " hours", i);
     }
     else if ((t % 60) == 0) {
          i = t / 60;
          if (i == 1)
               fprintf(fp,"1 minute");
          else
               fprintf(fp,"%" PRIu64 " minutes", i);
     }
     else {
          if (t == 1)
               fprintf(fp,"1 second");
          else
               fprintf(fp,"%" PRIu64 " seconds", t);
     }
}


/* hacked up tcpdump code
 * o simulates -tttt options
 * o prints ts at GMT
 */
void sysutil_printts(FILE * stream, time_t sec, time_t usec) {

     struct tm tm;
     time_t Time;
     register int s = sec % 86400;
     Time = sec - s;
     GMTIME_R(&Time, &tm);
     fprintf(stream,"%04d.%02d.%02d %02d:%02d:%02d.%06u",
             tm.tm_year+1900,
             tm.tm_mon+1, tm.tm_mday,
             s / 3600, (s % 3600) / 60,
             s % 60, (unsigned)usec);
}

int sysutil_snprintts(
          char* buf,
          size_t len,
          time_t sec,
          time_t usec)
{
     struct tm tm;
     time_t Time;
     register int s = sec % 86400;
     Time = sec - s;
     GMTIME_R(&Time, &tm);
     return snprintf(
          buf, len,
          "%04d.%02d.%02d %02d:%02d:%02d.%06u",
          tm.tm_year+1900,
          tm.tm_mon+1, tm.tm_mday,
          s / 3600, (s % 3600) / 60,
          s % 60, (unsigned)usec);
}

void sysutil_printts_sec(FILE * stream, time_t tsec) {
     struct tm tm;
     time_t Time;
     register int s = tsec % 86400;
     Time = tsec - s;
     GMTIME_R(&Time, &tm);
     fprintf(stream,"%04d.%02d.%02d %02d:%02d:%02d",
             tm.tm_year+1900,
             tm.tm_mon+1, tm.tm_mday,
             s / 3600, (s % 3600) / 60,
             s % 60);
}

int sysutil_snprintts_sec(char* buf, size_t len, time_t sec) {
     struct tm tm;
     time_t Time;
     register int s = sec % 86400;
     Time = sec - s;
     GMTIME_R(&Time, &tm);
     return snprintf(
          buf, len,
          "%04d.%02d.%02d %02d:%02d:%02d",
          tm.tm_year+1900,
          tm.tm_mon+1, tm.tm_mday,
          s / 3600, (s % 3600) / 60,
          s % 60);
}

int sysutil_snprintts_sec2(char* buf, size_t len, time_t sec) {
     struct tm tm;
     time_t Time;
     register int s = sec % 86400;
     Time = sec - s;
     GMTIME_R(&Time, &tm);
     return snprintf(
          buf, len,
          "%04d.%02d.%02d_%02d.%02d.%02d",
          tm.tm_year+1900,
          tm.tm_mon+1, tm.tm_mday,
          s / 3600, (s % 3600) / 60,
          s % 60);
}





int sysutil_file_exists(const char* filename) {
     struct stat statbuffer;
     //check filename
     if ((stat(filename, &statbuffer) == 0) &&
         S_ISREG(statbuffer.st_mode) &&
         (statbuffer.st_size > 0))
     {
          return 1;
     }
     return 0;
}


int set_sysutil_pConfigPath(const char *pConfPath) {
     if ( sysutil_pConfigPath ) free(sysutil_pConfigPath);
     sysutil_pConfigPath = strdup(pConfPath);
     if ( !sysutil_pConfigPath ) return 0;

     /* Process to build the Paths array */
     size_t p = 0;
     char * state;
     char * buf = sysutil_pConfigPath;
     char * tok = strtok_r(buf, ":", &state);
     while ( p <= MAX_PATH_COUNT && tok ) {
          sysutil_pConfigPaths[p++] = tok;
          tok = strtok_r(NULL, ":", &state);
     }

// TODO: looks like this source string could be leaked
     return 1;
}

int get_sysutil_pConfigPath(
          uint32_t pathComponent,
          char** buf,
          unsigned int* len)
{
     if ( pathComponent > MAX_PATH_COUNT ) return 0;

     if (sysutil_pConfigPaths[pathComponent] && *buf) {
          size_t nchars = 0;
          nchars = snprintf(*buf, *len, "%s", sysutil_pConfigPaths[pathComponent]);
          *len = strlen(*buf);
          return nchars;
      }
     return 0;
}

void free_sysutil_pConfigPath(void) {
     free(sysutil_pConfigPath);
}

int sysutil_prepend_config_path(char** fname) {
     if((*fname[0] == '.') || (*fname[0] == '/')) return 1;

     const char* old_path = strdup(*fname);
     if(!old_path) return 0;

    const size_t new_len = strlen(sysutil_pConfigPaths[0]) + strlen(old_path) + 2;

    *fname = (char*)realloc(*fname, new_len);
     if (!*fname) {
          error_print("failed sysutil_prepend_config_path realloc of *fname");
          free((void*)old_path);
          return 0;
     }

     snprintf(*fname, new_len, "%s/%s", sysutil_pConfigPaths[0], old_path);
     free((void*)old_path);

     return 1;
}

//wrapper for fopen to allow user to use default config file path..
FILE * sysutil_config_fopen(
          const char* fname,
          const char* opts)
{
     char buf[PATH_MAX];
     FILE * retval = NULL;

     //check if we don't have a fully qualified path..
     if (sysutil_pConfigPath && ((fname[0] != '.') && (fname[0] != '/')))
     {
          uint32_t p = 0;
          while ( p < MAX_PATH_COUNT && sysutil_pConfigPaths[p] ) {
               snprintf(buf, PATH_MAX, "%s/%s",
                        sysutil_pConfigPaths[p++], fname);
               if (sysutil_file_exists(buf)) {
                    retval = fopen(buf, opts);
                    return retval;
               }
          }
     }
     retval = fopen(fname, opts);
     return retval;
}

void sysutil_config_fclose(FILE * fp) {
     fclose(fp);
}

//look for | hex | values - to decode like snort does
int sysutil_decode_hex_escapes(char* str, size_t* len) {
     char scratchpad[SCRATCHPAD_LEN];
     char * startpipe;
     char * endpipe;
     char * strpos;
     char * strend;
     char * spad;
     size_t spad_len = 0;
     size_t i;

     // error check
     if (*len > SCRATCHPAD_LEN) {
          error_print("incoming string too long (%" PRIu64 ") for scratchpad (%u).", 
                      *len, SCRATCHPAD_LEN);
          return 0;
     }

     strpos = str;
     strend = str + (*len); // pointer math
     spad = scratchpad;

     // get string that starts at first pipe
     startpipe = (char *)memchr(strpos, '|', (strend - strpos));
     // no escapes ...
     if (startpipe == NULL) {
          return 0;
     }

     // get pointer to next pipe after first pipe
     endpipe = (char *)memchr(startpipe + 1, '|', (strend - startpipe));

     if (endpipe == NULL) {
          return 0;
     }

     // check that we have two pipes and the string is at least 2 characters
     while (startpipe && endpipe && (strpos < strend)) {
          // copy from working string all char from beginning to first pipe
          memcpy(spad, strpos, startpipe - strpos);

          spad += (startpipe - strpos); // move scratch pointer to just after first pipe
          spad_len += (startpipe - strpos); // update length
          if (endpipe < strend) { // more string that may need decoding
               strpos = endpipe + 1; // get pointer to after endpipe
          } else { // at end of string
               strpos = strend; // get pointer to end of string
          }

          startpipe++; // move after first pipe
          for (i = 0; i < (endpipe - startpipe); i++) {
               if (isxdigit(startpipe[i])) {
                    if (isxdigit(startpipe[i + 1])) {
                         spad[0] = (char)strtol(startpipe + i, NULL, 16);
                         spad++;
                         spad_len++;
                         i++;
                    }
               }
          }

          //find next pipes...
          startpipe = (char *)memchr(strpos, '|', (strend - strpos));

          //no escapes..
          if (startpipe) {
               endpipe = (char *)memchr(startpipe + 1, '|', (strend - startpipe));
          }
     }

     if (strend != strpos) {
          memcpy(spad, strpos, strend - strpos);
          spad_len += strend - strpos;
     }

     //copy scratchpad back to str..
     memcpy(str, scratchpad, spad_len);
     *len = spad_len;

     return 1;
}


