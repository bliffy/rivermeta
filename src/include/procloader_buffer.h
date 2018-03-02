#ifndef _PROCLOADER_BUFFER_H
#define _PROCLOADER_BUFFER_H

#include "waterslide.h"
#include "wstypes.h"
#include "datatypes/wsdt_tuple.h"

#ifdef __cplusplus
extern "C" {
#endif

int procbuffer_option(void*, void*, int, const char*);

//read in command line options
int procbuffer_init(void*, void*);

int procbuffer_decode(void*, wsdata_t*, wsdata_t*, uint8_t* buf, int len);

#ifdef __cplusplus
}
#endif

#endif // _PROCLOADER_BUFFER_H
