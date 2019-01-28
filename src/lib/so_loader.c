
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#include "waterslide.h"
#include "so_loader.h"
#include "init.h"
#include "listhash.h"
#include "mimo.h"

#include "dl_assist.h" // custom dynamic-lib macros
#include "dir_assist.h" // custom dir listing

// NOTE:  mimo->verbose is unset when modules and datatypes
// are loaded, which is before read_cmd_options is called.
// I recommend local control of verbosity, since we are
// rarely interested in this output.
// So set this item to nonzero if you want so_loader
// verbosity.
#define SO_LOADER_VERBOSE 0

// Globals
// These would be a problem, if more than one thread were
// loading modules or datatypes
DLHANDLE * stored_handles;
int num_fh = 0;

void print_dl_error(void) {
     char buf[1024]; buf[0]=0;
     DLERROR(buf,1024);
     error_print("proc dlopen %s", buf);
}

int ends_with(const char* subj, const char* suffix) {
     const size_t sfLen = strlen(suffix);
     const size_t sbLen = strlen(subj);
     if (sfLen > sbLen) return 0;
     return 0==strcmp(subj+(sbLen-sfLen),suffix);
}

// find the right .so filename to load as a datatype
int datatype_file_filter(const char * entry) {
     static const char* prefix = "wsdt_";
     size_t prefix_len = 5;
     if (0!=strncmp(entry, prefix, prefix_len))
          return 0;
     if (!ends_with(entry, WS_PROC_MOD_SUFFIX)) 
          return 0;
     return 1;
}

static int store_dlopen_file_handle(DLHANDLE sh_file_handle)
{
     if (!sh_file_handle)
          return 1;
     num_fh++;
     stored_handles = (DLHANDLE*)realloc(
          stored_handles,
          num_fh*sizeof(DLHANDLE));
     if (!stored_handles) {
          error_print("failed realloc of stored_handles");
          return 0;
     }
     stored_handles[num_fh-1] = sh_file_handle;
     return 1;
}

void free_dlopen_file_handles(void) {
     for (int i = 0; i < num_fh; i++) {
          DLCLOSE( stored_handles[i] );
     }
     free(stored_handles);
}

#define DATATYPELOADER_FUNC "datatypeloader_init"

typedef int (*datatypeloader_func)(void *);

// used as callback method for dir scanner
void load_datatype_library(
          const char * dir,
          const char * file,
          void * opaque)
{
     mimo_t * mimo = (mimo_t*)opaque;
     int dlen = strlen(dir);
     int flen = strlen(file);
     char * fullname = (char *)calloc(1, dlen+flen+2);
     if (!fullname) {
          error_print("failed load_datatype_library calloc of fullname");
          return;
     }

     memcpy(fullname, dir, dlen);
     fullname[dlen]= '/';
     memcpy(fullname + dlen+1, file, flen);

     //ok now we have filename to load..
     DLHANDLE sh_file_handle;
     if ( (sh_file_handle = DLOPEN(fullname)) ) {
          if (!store_dlopen_file_handle(sh_file_handle)) {
               return;
          }
          datatypeloader_func d_func;
          d_func = (datatypeloader_func) DLSYM(
               sh_file_handle,
               DATATYPELOADER_FUNC);

          if (d_func) {
               //launch it..
               d_func(&mimo->datalists);
#if SO_LOADER_VERBOSE
               status_print("loaded [%s]", fullname);
#endif
          }
          else {
               // this branch is encountered, for example,
               // when looking for hardware-architecture
               // dependent datatype on a non-conforming
               // architecture
#if SO_LOADER_VERBOSE
               error_print("load failed [%s]", fullname);
               print_dl_error();
#endif
          }
     }
     else {
          error_print("load failed [%s]", fullname);
          print_dl_error();
     }

     free(fullname);
     return;
}

int load_datatype_dir(mimo_t * mimo, const char * dirname) {
     int res = wsdir_scan(
          dirname,
          datatype_file_filter,
          load_datatype_library,
          (void*) mimo);
     if ( !res ) {
          error_print("no datatype path set");
          return 0;
     }
     return 1;
}

static void wsdatatype_init_sub(
          void * data,
          void * type_table)
{
     wsdatatype_t * dtype = (wsdatatype_t *) data;
     wssubelement_t * sub;
     for (int i = 0; i < dtype->num_subelements; i++) {
          sub = &dtype->subelements[i];
          if (!sub->dtype_name) {
               sub->dtype = NULL;
               continue;
          }
          sub->dtype = wsdatatype_get(
               type_table,
               sub->dtype_name);
          if (!sub->dtype) {
               error_print("unknown dtype in subelement %s:%s", sub->label->name, sub->dtype_name);
          }
     }
}

static void wsdatatype_init_subelements(mimo_t * mimo) {
     listhash_scour(
          mimo->datalists.dtype_table,
          wsdatatype_init_sub,
          &mimo->datalists);
}

#define PATHDELIM ":"
static int multiple_datapath_lookup(
          mimo_t * mimo,
          const char * datatype_path)
{
     char * dup = strdup(datatype_path);
     char * svptr;
     int rtn = 0;
     char * tok = strtok_r(dup, PATHDELIM, &svptr);
     while (tok) {
          size_t len = strlen(tok);
          if (len) {
               rtn += load_datatype_dir(mimo, tok);
          }
          tok = strtok_r(NULL, PATHDELIM, &svptr);
     }
     if (dup)
          free(dup);
     return rtn;
     
}

int load_datatype_libraries(mimo_t * mimo) {
     char * datatype_path = getenv(ENV_WS_DATATYPE_PATH);
     int rtn = 0;
     if (!datatype_path) {
          datatype_path="./datatypes";
#if SO_LOADER_VERBOSE
          error_print("need to set environment %s",
                      ENV_WS_DATATYPE_PATH);
          status_print("trying default %s...",
                       datatype_path);
#endif
          rtn = load_datatype_dir(mimo, datatype_path);
     }
     else {
#if SO_LOADER_VERBOSE
          status_print("datatype_path %s", datatype_path);
#endif
          rtn = multiple_datapath_lookup(mimo, datatype_path);
     }
     wsdatatype_init_subelements(mimo);
     return rtn;
}

#define WS_MAX_MODULES 1024

#define MAX_ALIAS_BUF 1000
#define ALIAS_TOK ", :"
void ws_proc_alias_open(
          mimo_t * mimo,
          const char * filename)
{
     FILE * fp = fopen(filename, "r");

     if (!fp) {
          error_print("ws_proc_alias_open input file %s could not be located", filename);
          error_print("Alias module not found");
          return;
     }

#if SO_LOADER_VERBOSE
     status_print("loading aliases from %s", filename);
#endif

     if (!mimo->proc_module_list) {
          mimo->proc_module_list = listhash_create(
               WS_MAX_MODULES,
               sizeof(ws_proc_module_t));
     }

     char buf[MAX_ALIAS_BUF];
     int buflen;
     char * tok;
     char * ptok;
     ws_proc_module_t * module;

     while (fgets(buf, MAX_ALIAS_BUF, fp)) {
          module = NULL;
          buflen = strlen(buf);
          //strip of return character
          if (buf[buflen -1]=='\n') {
               buflen--;
               buf[buflen] = 0;
          }
          tok = strtok_r(buf, ALIAS_TOK, &ptok);
          if (tok) {
               //module name
               dprint("alias: setting module %s", tok);
               module = listhash_find_attach(
                    mimo->proc_module_list,
                    tok,
                    strlen(tok));
               if (!module->name) {
                    module->name = strdup(tok);
               }
               module->strdup_set = 1;
          }
          if (!module) {
               continue;
          }
          //get aliases
          tok = strtok_r(NULL, ALIAS_TOK, &ptok);
          while (tok) {
               dprint("alias: setting alias %s", tok);
               listhash_find_attach_reference(
                    mimo->proc_module_list,
                    tok,
                    strlen(tok),
                    module);
               //get next alias
               tok = strtok_r(NULL, ALIAS_TOK, &ptok);
          }
     }

     fclose(fp);
}

static int so_load_wsprocbuffer(
          void * sh_file_handle,
          ws_proc_module_t * module)
{
     module->pbkid = (wsprocbuffer_kid_t*)calloc(
          1, sizeof(wsprocbuffer_kid_t));

     if (!module->pbkid) {
          error_print("failed so_load_wsprocbuffer calloc of module->pbkid");
          return 0;
     }

     module->pbkid->init_func =
          (wsprocbuffer_sub_init) DLSYM(sh_file_handle,"procbuffer_init");
     module->pbkid->option_func =
          (wsprocbuffer_sub_option) DLSYM(sh_file_handle,"procbuffer_option");
     module->pbkid->option_str =
          (char *) DLSYM(sh_file_handle,"procbuffer_option_str");
     module->pbkid->decode_func =
          (wsprocbuffer_sub_decode) DLSYM(sh_file_handle,"procbuffer_decode");
    
     module->pbkid->element_func =
          (wsprocbuffer_sub_element) DLSYM(sh_file_handle,"procbuffer_element");

     module->pbkid->destroy_func =
          (wsprocbuffer_sub_destroy) DLSYM(sh_file_handle,"procbuffer_destroy");

     module->pbkid->labeloffset =
          (proc_labeloffset_t *) DLSYM(sh_file_handle,"proc_labeloffset");

     module->pbkid->name = (char *) DLSYM(sh_file_handle,"proc_name");

     if (!module->pbkid->option_str) {
          module->pbkid->option_str = "h";
     }

     int * npass;
     npass = (int*)DLSYM(sh_file_handle,"procbuffer_pass_not_found");
     if (npass) {
          module->pbkid->pass_not_found = *npass;
     }

     int * isize;

     isize = (int *)DLSYM(sh_file_handle,"procbuffer_instance_size");
     dprint("instance size %d", *isize);

     if (isize) {
          module->pbkid->instance_len = *isize;
     }

     module->proc_init_f = NULL;
     module->proc_input_set_f = wsprocbuffer_input_set;
     module->proc_destroy_f = wsprocbuffer_destroy;
     if (!module->name) {
          module->name = (char *) DLSYM(sh_file_handle,"proc_name");
          module->pbkid->name = module->name;
     }

     return 1;
}

static int so_load_wsprockeystate(
          void * sh_file_handle,
          ws_proc_module_t * module)
{
     module->kskid = (wsprockeystate_kid_t*)calloc(
          1, sizeof(wsprockeystate_kid_t));
     if (!module->kskid) {
          error_print("failed so_load_wsprockeystate calloc of module->kskid");
          return 0;
     }

     module->kskid->init_func =
          (wsprockeystate_sub_init) DLSYM(sh_file_handle,"prockeystate_init");
     module->kskid->option_func =
          (wsprockeystate_sub_option) DLSYM(sh_file_handle,"prockeystate_option");
     module->kskid->option_str =
          (char *) DLSYM(sh_file_handle,"prockeystate_option_str");
     module->kskid->update_func =
          (wsprockeystate_sub_update) DLSYM(sh_file_handle,"prockeystate_update");
     module->kskid->force_expire_func =
          (wsprockeystate_sub_force_expire)
          DLSYM(sh_file_handle,"prockeystate_force_expire");
     module->kskid->update_value_func =
          (wsprockeystate_sub_update_value)
          DLSYM(sh_file_handle,"prockeystate_update_value");
     module->kskid->expire_func =
          (wsprockeystate_sub_expire)
          DLSYM(sh_file_handle,"prockeystate_expire");
     module->kskid->flush_func =
          (wsprockeystate_sub_flush)
          DLSYM(sh_file_handle,"prockeystate_flush");
     module->kskid->destroy_func =
          (wsprockeystate_sub_destroy)
          DLSYM(sh_file_handle,"prockeystate_destroy");

     module->kskid->labeloffset =
          (proc_labeloffset_t *)
          DLSYM(sh_file_handle,"proc_labeloffset");

     module->kskid->name = (char *) DLSYM(sh_file_handle,"proc_name");


     if (!module->kskid->option_str) {
          module->kskid->option_str = "";
     }

     int * isize;
     isize = (int *)DLSYM(sh_file_handle,"prockeystate_instance_size");
     dprint("instance size %d", *isize);

     if (isize) {
          module->kskid->instance_len = *isize;
     }

     int * state_size;
     state_size = (int *)DLSYM(sh_file_handle,"prockeystate_state_size");
     dprint("state size %d", *state_size);

     if (state_size) {
          module->kskid->state_len = *state_size;
     }

     module->proc_init_f = NULL;
     module->proc_input_set_f = wsprockeystate_input_set;
     module->proc_destroy_f = wsprockeystate_destroy;
     if (!module->name) {
          module->name = (char *) DLSYM(sh_file_handle,"proc_name");
          module->kskid->name = module->name;
     }

     return 1;
}

#define DIRSEP_STR ":"
#define DIRSEP_CHR ':'
static mimo_directory_list_t * build_kid_dirlist(void) {
     char * envlist = getenv(ENV_WS_PROC_PATH);
     if (!envlist) {
          envlist = "./procs";
     }
     char * dstr = strdup(envlist);
     //find out how big a list
     char * ptr = dstr;

     mimo_directory_list_t * dlist = calloc(
          1, sizeof(mimo_directory_list_t)); 
     if (!dlist) {
          error_print("failed build_kid_dirlist calloc of dlist");
          return NULL;
     }

     dlist->len = 1;

     while((ptr = (char *)strchr(ptr, (int)DIRSEP_CHR)) != NULL) {
          dlist->len++;
          ptr++;
     }

     dlist->directories = calloc(dlist->len, sizeof(char *));
     if (!dlist->directories) {
          error_print("failed build_kid_dirlist calloc of dlist->directories");
          return NULL;
     }

     int j = 0;
     char * buf = dstr;
     char * svptr;
     char * dir = strtok_r(buf, DIRSEP_STR, &svptr);

     while (dir) {
          dlist->directories[j] = dir;
          int len = strlen(dir);
          dprint("adding kid directory path %s", dir);
          if (len > dlist->longest_path_len) {
               dlist->longest_path_len = len;
          }
          j++;
          if (j == dlist->len) {
               break;
          }
          dir = strtok_r(NULL, DIRSEP_STR, &svptr);
     }
     return dlist;
}



ws_proc_module_t * ws_proc_module_dlopen(
          mimo_t * mimo,
          const char * fullname,
          const char * modname)
{
     ws_proc_module_t * module;
     DLHANDLE sh_file_handle;
     int i;

     sh_file_handle = DLOPEN(fullname);
     
     if (!sh_file_handle) {
          print_dl_error();
          return NULL;
     }
     if(!store_dlopen_file_handle(sh_file_handle)) {
          return NULL;
     }

     if (!mimo->proc_module_list) {
          mimo->proc_module_list = listhash_create(
               WS_MAX_MODULES,
               sizeof(ws_proc_module_t));
     }
     module = listhash_find_attach(
          mimo->proc_module_list,
          modname,
          strlen(modname));

     int * pb = (int *) DLSYM(
          sh_file_handle,"is_procbuffer");
     int * ks = (int *) DLSYM(
          sh_file_handle,"is_prockeystate");
     if (pb) {
          if(!so_load_wsprocbuffer(sh_file_handle, module)) { 
               return NULL;
          }
     }
     else if (ks) {
          if(!so_load_wsprockeystate(sh_file_handle, module)) { 
               return NULL;
          }
     }
     else {
          module->proc_init_f = (proc_init_t) DLSYM(
               sh_file_handle,"proc_init");
          module->proc_init_finish_f =
               (proc_init_finish_t)
               DLSYM(sh_file_handle,"proc_init_finish");
          module->proc_input_set_f =
               (proc_input_set_t)
               DLSYM(sh_file_handle,"proc_input_set");
          module->proc_destroy_f =
               (proc_destroy_t)
               DLSYM(sh_file_handle,"proc_destroy");
          if (!module->name) {
               module->name = (char *) DLSYM(
                    sh_file_handle,
                    "proc_name");
          }
     }

     if (!module->name) {
          error_print("module has no name");
          return NULL;
     }

     module->did_init = 1;

     int * dep = (int *)DLSYM(sh_file_handle,"is_deprecated");
     if ( dep ) mimo_using_deprecated(mimo, module->name);

     dprint("here in dlopen");

     listhash_find_attach_reference(
          mimo->proc_module_list,
          module->name,
          strlen(module->name),
          module);

     dprint("here in dlopen2");

     module->aliases =
          (char **) DLSYM(sh_file_handle,"proc_alias");


     dprint("here in dlopen3");
     
     //walk aliases .. add to module list
     if (module->aliases) {
          i = 0;
          while(module->aliases[i]) {
               dprint("alias %s, %s", module->name, module->aliases[i]);
               listhash_find_attach_reference(
                    mimo->proc_module_list,
                    module->aliases[i],
                    strlen(module->aliases[i]),
                    module);
               i++;
          }
     }

     return module;
}

static char * find_kid_fullpath(
          const char * modname,
          mimo_directory_list_t * dlist)
{
     //allocate buffer for flength
     int totallen = (
          dlist->longest_path_len
          + strlen(modname)
          + strlen(WS_PROC_MOD_PREFIX)
          + strlen(WS_PROC_MOD_SUFFIX)
          + 5 );

     char * fullname = (char *)calloc(1, totallen);
     if (!fullname) {
          error_print("failed find_kid_fullpath calloc of fullname");
          return NULL;
     }

     struct stat statbuffer;

     for (int i = 0; i < dlist->len; i++) {
          snprintf(fullname, totallen, "%s%s%s%s",
                   dlist->directories[i],
                   WS_PROC_MOD_PREFIX,
                   modname,
                   WS_PROC_MOD_SUFFIX);
          dprint("attemping kid path %s", fullname);

          if (!stat(fullname, &statbuffer)) {
               // we have a file
               return fullname;
          }
     }
     free(fullname);
     return NULL;

}

ws_proc_module_t * ws_proc_module_find(
          mimo_t * mimo,
          const char * modname)
{
     ws_proc_module_t * module = NULL;

     //see if module is already in list
     if (mimo->proc_module_list) {
          module = listhash_find(
               mimo->proc_module_list,
               modname,
               strlen(modname));     
          if (module) {
               if (module->did_init) {
                    return module;
               }
               else if (module->name) {
                    if (strcmp(module->name, modname) != 0) {
                         if (SO_LOADER_VERBOSE)  {
                              status_print("using %s for alias %s", module->name,
                                           modname);
                         }
                    }
                    modname = module->name;
               }
          }
     }

     if (!mimo->kid_dirlist) {
          mimo->kid_dirlist = build_kid_dirlist();
          if (!mimo->kid_dirlist) {
               return NULL;
          }
     }

     //get environment list .. try for each possible path

     //find module
     char * fullname = find_kid_fullpath(
          modname,
          mimo->kid_dirlist);
     if (!fullname) {
          return NULL;
     }

     //test file here.. with stat

     module = ws_proc_module_dlopen(mimo, fullname, modname);     
     free(fullname);
     return module;
}
