#ifndef _SO_LOADER_H
#define _SO_LOADER_H

#include "waterslide.h"

#ifdef __cplusplus
extern "C" {
#endif

void free_dlopen_file_handles(void);

int load_datatype_libraries(mimo_t*);

ws_proc_module_t * ws_proc_module_find(mimo_t*, const char*);

void ws_proc_alias_open(mimo_t*, const char* /*filename*/);

#ifdef __cplusplus
}
#endif

#endif // _SO_LOADER_H
