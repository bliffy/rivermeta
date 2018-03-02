#ifndef _PROCLOADER_KEYSTATE_H
#define _PROCLOADER_KEYSTATE_H

#include "waterslide.h"
#include "wstypes.h"
#include "datatypes/wsdt_tuple.h"

#ifdef __cplusplus
extern "C" {
#endif

int prockeystate_option(void*, void*, int, const char*);

//read in command line options
int prockeystate_init(void*, void*, int);

int prockeystate_update(void*, void*, wsdata_t*, wsdata_t*);
int prockeystate_update_value(
          void*, void*, wsdata_t*, wsdata_t*, wsdata_t*);
void prockeystate_expire(
          void*, void*, ws_doutput_t*, ws_outtype_t*);
void prockeystate_flush(void*);

#ifdef __cplusplus
}
#endif

#endif // _PROCLOADER_KEYSTATE_H
