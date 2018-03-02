#ifndef _PROCLOADER_H
#define _PROCLOADER_H

#include "waterslide.h"
#include "wstypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//read in command line options
int proc_init(wskid_t* kid, int, char**, void**, ws_sourcev_t*, void*);

//set input function.. assign output types
proc_process_t proc_input_set(
          void*,
          wsdatatype_t*,
          wslabel_t*,
          ws_outlist_t*,
          int,
          void*);

// graceful exit
int proc_destroy(void*);

#ifdef __cplusplus
}
#endif

#endif // _PROCLOADER_H
