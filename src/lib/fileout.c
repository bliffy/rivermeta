//#define DEBUG 1

#include "fileout.h"
#include "time_macros.h" // GMTIME_R()
#include <fcntl.h>

// cross platform macros
#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
#include <windows.h>
#include <direct.h> // defines _mkdir()
#define _MKDIR(x) (_mkdir(x)==ENOENT ? 1 : 0)
//#define _LINK(x,y) (0!=CreateSymbolicLinkA(x,y,0))
#define _LINK(x,y) (0!=CreateHardLinkA(x,y,0))
#define _UNLINK(x) DeleteFile(x)
#else
#define _MKDIR(x) ( \
     (mkdir(x,S_IRWXU|S_IRWXG|S_IRWXO)==0 || errno==EEXIST))
#define _LINK(x,y) (-1!=link(x,y))
#define _UNLINK(x) unlink(x)
#endif

//function prototypes for local functions
static void close_fp_callback(void *data, void *fs);
static void evict_fp_callback(void *data, void *fs);
static void close_fp(fpdata_t *f, filespec_t *fs, int eviction);
char *make_basename(filespec_t *, wsdata_t *, time_t expandtime);
void *open_file(fpdata_t *fp, filespec_t *fs);
int make_path(char *path);

fpdata_t *
fileout_initialize(filespec_t *fs, void *type_table) 
{
     
     fs->outfpdata = calloc(1, sizeof(fpdata_t));
     if (fs->outfp == stdout) {
       fs->outfpdata->fp = fs->outfp;
       return fs->outfpdata;
     }
     fs->outfpdata->mode = fs->mode;
     if (fs->timespec || fs->recordmax || fs->bytemax || fs->labelname) {
       fs->dynamicfile = 1;
     }

     if (fs->fileprefix) {
       if (!make_path(fs->fileprefix)) { 
            error_print("Can't write to destination %s", fs->fileprefix);
               free(fs->outfpdata);
            trigger_clean_exit(1);
            return NULL;
       }
     }
     if (fs->moveprefix) { 
       if (!make_path(fs->moveprefix)) {
            // somewhat specious, make_path() will punt upon failure
            error_print("Can't write to destination %s", fs->moveprefix);
               free(fs->outfpdata);
            trigger_clean_exit(1);
            return NULL;
       }
     }


     if (!fs->dynamicfile) {
       fs->outfpdata->filename = make_basename(fs, NULL, 0);
       open_file(fs->outfpdata, fs);
       if (fs->outfpdata->fp == 0) {
               free(fs->outfpdata);
               fs->outfpdata = NULL;
               return NULL;
          }
       return fs->outfpdata;
     }

     if (fs->labelname) {
       // get ready for having a table of FPs
       // note: size could be determined by getrlimit(RLIMIT_NOFILE...) 
       fs->label = wssearch_label(type_table, fs->labelname);

          // NOTE:  the following is not threadsafe - must be serial table only
       fs->fp_table = stringhash5_create(0, 512, sizeof(fpdata_t));
          stringhash5_set_callback(fs->fp_table, close_fp_callback, fs);
       fs->evicted_table = stringhash5_create(0, 4096, sizeof(fpdata_t));
          stringhash5_set_callback(fs->evicted_table, evict_fp_callback, fs);
     }

     return fs->outfpdata;
}


// =======================================

char *
sanitize (char * str)
{
     int i;
     int len = strlen(str);
     uint8_t safe[128] = {
       //0  1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
       0,  '_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
       '_','_','_','_','_','_','_','_','_','_','_','_','_','_','_','_',
       '_','_','_','#','$','%','_','_','_','_','_','+',',','-','.','_',
       '0','1','2','3','4','5','6','7','8','9',':','_','_','=','_','_',
       '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
       'P','Q','R','S','T','U','V','W','X','Y','Z','_','_','_','^','_',
       '_','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
       'p','q','r','s','t','u','v','w','x','y','z','_','_','_','_','_'};
     
     for (i = 0; i < len; i++) {
       str[i] = safe[str[i]&0x7f];
     }
     return str;
}

char *
sanitize_path(char *instr)
{
     char *ptr = instr;
     while ((ptr = strstr(ptr, ".."))) {
       *ptr++ = '_';
       *ptr++ = '_';
     }
     return instr;
}

char *
makestring(char *start, char *end) {
     int len = end-start;
     char *result = calloc(1,len+1);
     return memcpy(result, start, len);
}

char *
expand_env_vars(char *inputstr, int only_env) {

     char *ptr = inputstr;
     char *result;
     char *resultptr;
     int len;
     char *envname;
     char *envval;
     char *endptr;
     static int usernotified = 0;

     result = calloc(1, MAX_NAME_LEN);
     resultptr = result;
     len = strlen(inputstr);
     ptr = inputstr;

     while ((ptr - inputstr) < len) {
       switch (*ptr) {
       case '{': // env var
            endptr = strchr(ptr, '}');
            if (!endptr) {  // bad env var
              error_print("bad environment variable reference in %s", 
                    inputstr);
              return NULL;
            }
            
            envname = makestring(ptr+1, endptr);
            envval = getenv(envname);
            if (envval) {
              strcat(result,envval);
              resultptr += strlen(envval);
            } else {
              if (!usernotified) {
                tool_print("Note: environment variable %s not set; using {%s} as placeholder", envname, envname);
                usernotified = 1; // don't flood the user
              }
              strcat(result,"{");
              strcat(result,envname);
              strcat(result,"}");
              resultptr += strlen(envname) + 2;
            }
            ptr = endptr + 1;
            free(envname);
            break;
       case '[':
       case '<':
            if (only_env) {
              tool_print("No expansion of label values or timespecs in this field: %s", inputstr);
            }
            *resultptr++ = *ptr++;
            break;
       default:
            *resultptr++ = *ptr++;
            break;
       }
     }
     return result;
}

// Note: if called witn only_env, a char* is returned; otherwise, a 
//  filespec* is the return type

void *
fileout_parse_filespec(char *inputstr, filespec_t *fs, int only_env) {

     char *ptr;
     char *result;
     char *endptr;
     char *env_expanded;
     char *resultptr;
     int len;
     char *envname;
     char *envval;
     char *tempstr;
     static int usernotified = 0;

     env_expanded = expand_env_vars(inputstr, only_env);

     if (only_env) {
       return env_expanded;
     }

     ptr = env_expanded;
     result = calloc(1, MAX_NAME_LEN);
     resultptr = result;
     len = strlen(env_expanded);
     fs->outfp = NULL;

     while ((ptr - env_expanded) < len) {
       switch (*ptr) {
       case '\0': // end
            if (result[0]) {
              fs->namepiece[fs->namepiece_count++] = strdup(result);
            }
            return fs;
            break;

// Note that an environment variable can reference another environment variable
//   Only one extra layer, not turtles all the way down...

       case '{': // env var
            endptr = strchr(ptr, '}');
            if (!endptr) {  // bad env var
              error_print("bad environment variable reference in %s", inputstr);
              return NULL;
            }
            
            envname = makestring(ptr+1, endptr);
            envval = getenv(envname);
            if (envval) {
              strcat(result,envval);
              resultptr += strlen(envval);
            } else {
              if (!usernotified) {
                tool_print("Note: environment variable %s not set; using {%s} as placeholder", envname, envname);
                usernotified = 1; // Don't flood the user
              }
              strcat(result,"{");
              strcat(result,envname);
              strcat(result,"}");
              resultptr += strlen(envname) + 2;
            }
            ptr = endptr + 1;
            free(envname);
            break;

       case '[': // label
            if (fs->labelname) {
              error_print("only one label per filespec (%s)", inputstr);
              return NULL;
            }
            fs->dynamicfile = 1;
            endptr = strchr(ptr, ']');
            if (!endptr) {  // bad label notation
              error_print("bad label notation in %s", inputstr);
              return NULL;
            }
            if (result[0]) {
              // save partial answer
              fs->namepiece[fs->namepiece_count++] = strdup(result);
              memset(result, 0, MAX_NAME_LEN);
              resultptr = result;
            }
            tempstr = makestring(ptr, endptr);
            fs->namepiece[fs->namepiece_count++] = tempstr; 
            fs->labelname = tempstr + 1;
            ptr = endptr + 1;
            break;       

       case '<': // timestamp
            if (fs->timespec) {
              error_print("only one timestamp per filespec (%s)", inputstr);
              return NULL;
            }
            fs->dynamicfile = 1;
            endptr = strchr(ptr, '>');
            if (!endptr) { // bad timestamp
              error_print("bad timestamp in %s", inputstr);
              return NULL;
            }
            if (result[0]) {
              // save partial answer
              fs->namepiece[fs->namepiece_count++] = strdup(result);
              memset(result, 0, MAX_NAME_LEN);
              resultptr = result;
            }
            if (endptr == ptr + 1) { // default timespec
              fs->namepiece[fs->namepiece_count++] = strdup("<%Y%m%d.%H%M");
              fs->timespec = "%Y%m%d.%H%M";
            } else {
              tempstr = makestring(ptr, endptr);
              fs->namepiece[fs->namepiece_count++] = tempstr;
              fs->timespec = tempstr+1;
            }
            ptr = endptr + 1;
            break;
            
       default:
            *resultptr++ = *ptr++;
            break;
       }
     }            
     if (result[0]) {
       fs->namepiece[fs->namepiece_count++] = strdup(result);
     }
     if (!fs->timeslice) { 
       fs->timeslice = sysutil_get_duration_ts("1h");
     }
     free(env_expanded);
     free(result);

     return fs;
}

char * make_basename(
          filespec_t * fs,
          wsdata_t * input,
          time_t basetime)
{
     int i=0;
     char * result = calloc(1,MAX_NAME_LEN);
     wsdata_t ** mset;
     int mset_len;
     const char * string;
     size_t len_str;

     for (i=0; i < fs->namepiece_count; i++) {
          switch(fs->namepiece[i][0]) {
          case '[': // process label
               if (input && tuple_find_label(input, fs->label, &mset_len, &mset)) {
                    if (mset[0]->dtype->to_string(mset[0], &string, &len_str)) {
                         size_t at = strlen(result);
                         if (len_str) {
                              memcpy(result+at,
                                     string,
                                     len_str);
                         }
                         result[at+len_str] = '\0';
                         sanitize(result+at);
                         break;
                    }               
               }
               // If label not found or it's not printable
               strcat(result, "[");
               strcat(result, fs->labelname);
               strcat(result, "]");
               break;

          case '<': // process time
               if (!basetime) { 
                    // placeholder; perhaps not needed
                    strcat(result, "timestamp");
               } else {
                    char tmptm[300];
                    struct tm tres;
                    GMTIME_R(&basetime, &tres);
                    strftime(
                         tmptm, 300,
                         &fs->namepiece[i][1],
                         &tres);
                    strcat(result, tmptm);
                    break;
               }
               break;
          default:
               strcat(result, fs->namepiece[i]);
               break;
          }
     }
     return result;
}

int
make_path(char *name)
{
     char *temp = strdup(name);
     // let's see if the directory already exists or we can easily create it...
     char *ptr = strrchr(temp,'/');
     if (ptr) { 
       *ptr = 0;
       if ( _MKDIR(temp) ) {
            free(temp);
            return 1;
       }
       *ptr = '/';
     }

     // If not, we'll walk the path laying the planks as we go
     ptr = strchr(temp,'/');
     while (ptr) {
       if (ptr == temp) { // filespec starts at root
            ptr = strchr(ptr+1,'/'); 
            continue;
       }
       *ptr = 0;

       if ( _MKDIR(temp) ) {
            // keep looping
       } else {
            error_print("Can't create directory %s (%s)", 
                  temp, strerror(errno));
            trigger_clean_exit(1);
            return 0;
       }            
       *ptr = '/';
       ptr = strchr(ptr+1,'/');
     }
     free(temp);
     return 1;
}

void * 
open_file(fpdata_t *fpd, filespec_t *fs) 
{
     char realname[MAX_NAME_LEN] = "";
     char fullname[MAX_NAME_LEN];
     char mode[5] = "";
     int versionoffset;
     int namelen;
     int tempfd=0;
           
     strcpy(realname, fpd->filename);
     if (fpd->rollovercount != 0) {
       sprintf(realname+strlen(realname), "_pt%03u",fpd->rollovercount);
     }

     versionoffset = strlen(realname);
     // include a version if there is one....
     if (fpd->version != 0) {
       sprintf(realname+versionoffset, "_v%03u", fpd->version);
     }
     namelen = strlen(realname); // size w/o extension

     sprintf(fullname,"%s%s%s", 
          fs->fileprefix ? fs->fileprefix : "" ,
          realname,
          fs->extension ? fs->extension : "");

     make_path(fullname);

     mode[0] = fpd->mode;
     if (fpd->mode == 'w') { 
       if (fs->safename) { 
            while ((tempfd = open(fullname,O_CREAT|O_EXCL|O_WRONLY, 0644)) == -1) {
              if (fpd->version > MAX_FILE_VERSIONS) {
                error_print("file version count exceeded");
                return 0;
              }
              
              namelen = sprintf(realname+versionoffset,"_v%03u", ++fpd->version);
              namelen += versionoffset;  // keep track of filename len w/o extension
              sprintf(fullname,"%s%s%s", 
                   fs->fileprefix ? fs->fileprefix : "" ,
                   realname,
                   fs->extension ? fs->extension : "");
            }
       } else { // overwrite any existing file
            tempfd = open(fullname, O_CREAT | O_WRONLY, 0644);
       }
     } else { // append mode
       tempfd = open(fullname, O_CREAT | O_APPEND | O_WRONLY, 0644);
     }

     dprint("opening %s (mode: %c)", fullname, fpd->mode);

     // transform to FILE* equivalent
     if (fs->use_gzip) {       
       strcat(mode, "b9");
       fpd->fp = gzdopen(tempfd, mode);
     } else {
       fpd->fp = fdopen(tempfd, mode);
     }
     if (!fpd->fp) { 
       error_print("fileout open_file can't open file %s (%s)", fullname, strerror(errno));
       return 0;
     }
     fpd->expandedname = strdup(realname);
     fpd->expandedname[namelen] = '\0'; // no extension stored
     return fpd->fp;
}

static void
evict_fp_callback(void *data, void *fs) 
{
     fpdata_t *fpdata = data;
     dprint("expiry of %s\n", fpdata->filename);
     free(fpdata->filename);
}

static void 
close_fp_callback(void *data, void *filespec)
{
// callback for when we expire an entry
// will get re-opened in append mode if re-encountered
     fpdata_t *fpdata = data;
     filespec_t *fs = filespec;
     if (fpdata->fp) {
       fpdata->mode = 'a';
       close_fp(fpdata, fs, 1);
     } 
} 

 
static void 
close_fp(fpdata_t *fpd, filespec_t *fs, int eviction) {
    
     fpdata_t *old_fp;

     if (fs->use_gzip) {
       gzclose(fpd->fp);
     } else {
       fclose(fpd->fp);
     }
     fpd->fp = 0;
// move file if so directed...
     if (fs->moveprefix) {
       char currentname[MAX_NAME_LEN] = "";
       char finalname[MAX_NAME_LEN] = "";
       int copyversion = 0;
       int namelen;

       sprintf(currentname,"%s%s%s", 
            fs->fileprefix ? fs->fileprefix : "" ,
            fpd->expandedname, 
            fs->extension ? fs->extension : "");

       sprintf(finalname,"%s%s", fs->moveprefix,fpd->expandedname);
       namelen = strlen(finalname);
       if (fs->extension) {
            strcat(finalname, fs->extension);
       }

       if (!make_path(finalname)) {
            error_print("can't make path to %s\n",finalname);
            trigger_clean_exit(1);
       } else {
            if (! _LINK(currentname, finalname) ) {
              if (errno == EEXIST) {
              // dest file already exists
                do {
                     sprintf(finalname+namelen,"_x%03u",++copyversion);
                     if (fs->extension) { strcat(finalname, fs->extension); }
                } while ((!_LINK(currentname, finalname) && (errno == EEXIST)) 
                      && copyversion < MAX_FILE_VERSIONS);
                if (copyversion >= MAX_FILE_VERSIONS) {
                     error_print("can't find a unique name for %s across %d options\n", finalname, copyversion);
                     trigger_clean_exit(1);
                     return;
                }
/*   Could do this, but would take more testing
              } else if (errno == EXDEV) { 
                // target on different file system; copy it
                int src;
                int dst;
                tool_print("%s on different file system than %s; copying the file...",finalname, currentname);
                dst = open(finalname, O_WRONLY);
                src = open(currentname, O_RDONLY);
                char buf[8192];
                int result;
                if ((dst < 0) || (src < 0)) {
                     error_print("issue copying %s to %s: %s",
                           currentname, finalname, strerror(errno));
                     trigger_clean_exit(1);
                }
                while ((result = read(src, buf, sizeof(buf)))) {
                     write(dst, buf, sizeof(buf));
                }
                close(src);
                close(dst);
*/
              } else {
                // something else untoward happened
                error_print("problem moving %s to %s: %s",
                         currentname, finalname, strerror(errno));
                trigger_clean_exit(1);
              }
            }
            _UNLINK(currentname);
       }
     }
     if (fpd->expandedname) {
       free(fpd->expandedname);
     }

     if (eviction) {
       fs->evicted_flag = 1;
       // put into evicted table so that rollover is maintained
       if (fs->moveprefix) {
            fpd->rollovercount++;
       } else {
            fpd->mode = 'a';
       }
       old_fp = stringhash5_find_attach(fs->evicted_table, fpd->filename, strlen(fpd->filename));
       memcpy(old_fp, fpd, sizeof(fpdata_t));
       // note that it inherits the filename string
     } else {
       free(fpd->filename);
       fpd->version = 0;
     }
}


fpdata_t *
file_cycle(fpdata_t *fpdata, wsdata_t *input, filespec_t *fs, time_t tm)
{
     time_t currenttime = 0;

     if (fpdata->fp && (fs->timespec == 0) && 
      (fs->recordmax == 0) && (fs->bytemax == 0)) {
       // no time, recordcount or bytecount cycle
       return fpdata;
     }
     
     // deal with timecycle first...
     if (fs->timespec) {
       if (!tm) {
            tm = time(NULL); // get current time
       }
       currenttime = tm - (tm % fs->timeslice);
     }
     if (fpdata->ts != currenttime) {
       // all the "what if" records are now obsolete
       if (fs->evicted_table && fs->evicted_flag) {
            stringhash5_flush(fs->evicted_table);
            fs->evicted_flag = 0;
       }
     } else {
       if (fpdata->fp) {
            // now see if max values have been reached
            if (fs->recordmax && 
             (fpdata->recordcount >= fs->recordmax)) {
              close_fp(fpdata, fs, 0);
              fpdata->fp = 0;
              fpdata->rollovercount++;
            } else {
              if (fs->bytemax && 
               (fpdata->bytecount >= fs->bytemax)) {
                close_fp(fpdata, fs, 0);
                fpdata->fp = 0;
                fpdata->rollovercount++;
              } else {
                return fpdata;
              }
            }
       }
     }
     if (fpdata->fp) {
       close_fp(fpdata, fs, 0);
     }
     fpdata->filename = make_basename(fs, input, currenttime);
     fpdata->mode = fs->mode;
     open_file(fpdata, fs);
     if (!fpdata->fp) {
       error_print("Can't open %s (%s)", fpdata->filename, strerror(errno));
       trigger_clean_exit(1);
       return NULL;
     }
     fpdata->ts = currenttime;
     fpdata->recordcount = 0;
     fpdata->bytecount = 0;
     return fpdata;
}

fpdata_t *
fileout_select_file(wsdata_t *input, filespec_t *fs, time_t tm)
{
     char *outfile;
     fpdata_t *fpptr;
     fpdata_t *newfpptr;
     
     // if it's a static output file...
     if (!fs->dynamicfile) { 
       fs->outfpdata->recordcount++;
       return fs->outfpdata;
     }

     // if there's no label...
     if (!fs->labelname) {
       file_cycle(fs->outfpdata, NULL, fs, tm);
       fs->outfpdata->recordcount++;
       return fs->outfpdata;
     }

     // if there's a label...
     outfile = make_basename(fs, input, 0);
     fpptr = stringhash5_find(fs->fp_table, outfile, strlen(outfile));
     if (fpptr) {  
       if (strcmp(outfile, fpptr->filename) != 0) {
            dprint("hash collision: %s != %s\n", outfile, fpptr->filename);
            fpptr = NULL;
       } else {
            // found one...
            file_cycle(fpptr, input, fs, tm);
            fpptr->recordcount++;
            return fpptr;
       } 
     }
     // make a new entry...
     newfpptr = stringhash5_find_attach(fs->fp_table, outfile, strlen(outfile));

     // has it been previously evicted?
     fpptr = stringhash5_find(fs->evicted_table, outfile, strlen(outfile));
     if (fpptr) { 
       memcpy(newfpptr, fpptr, sizeof(fpdata_t));
       // note this inherits the filename string
       stringhash5_delete(fs->evicted_table, outfile, strlen(outfile));
     }
     // add fp entry to hash table...
     newfpptr = file_cycle(newfpptr, input, fs, tm);
     if (!newfpptr->fp) {
       // graceful exit; actually handled by file_cycle
       trigger_clean_exit(1);
       return NULL;
     }
     newfpptr->recordcount++;
     return newfpptr;
}

void 
fileout_filespec_cleanup(filespec_t *fs) {
     //clean up outfile(s)
     if (!fs) return;

     int i;
     for (i = 0; i < fs->namepiece_count; i++) {
           if (fs->namepiece[i]) {
                 free(fs->namepiece[i]);
           }
     }

     if (fs->fp_table) {
       stringhash5_scour(fs->fp_table, close_fp_callback, fs);
       stringhash5_destroy(fs->fp_table);
       stringhash5_destroy(fs->evicted_table);
     } else {
       if (fs->outfpdata->fp && (fs->outfpdata->fp != stdout)) {
            close_fp(fs->outfpdata, fs, 0);
       }
     }
     if (fs->outfpdata) {
          free(fs->outfpdata);
          fs->outfpdata = NULL;
     }
}

