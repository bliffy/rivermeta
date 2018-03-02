#ifndef _WSPROCKEYSTATE_H
#define _WSPROCKEYSTATE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <assert.h>
#include "waterslide.h"
#include "waterslidedata.h"
#include "datatypes/wsdt_tuple.h"
#include "wstypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*wsprockeystate_sub_init)(void *, void *, int);
typedef int (*wsprockeystate_sub_option)(
     void *,
     void *,
     int,
     const char *);
typedef int (*wsprockeystate_sub_update)(
     void *,
     void *,
     wsdata_t *,
     wsdata_t *);
typedef int (*wsprockeystate_sub_force_expire)(
     void *,
     void *,
     wsdata_t *,
     wsdata_t *);
typedef int (*wsprockeystate_sub_update_value)(
     void *,
     void *,
     wsdata_t *,
     wsdata_t *,
     wsdata_t *);
typedef void (*wsprockeystate_sub_expire)(
     void *,
     void *,
     ws_doutput_t *,
     ws_outtype_t *);
typedef void (*wsprockeystate_sub_flush)(void *);
typedef int (*wsprockeystate_sub_destroy)(void *);

typedef struct _wsprockeystate_kid_t {
     wsprockeystate_sub_init init_func;
     wsprockeystate_sub_option option_func;
     wsprockeystate_sub_update update_func;
     wsprockeystate_sub_update_value update_value_func;
     wsprockeystate_sub_force_expire force_expire_func;
     wsprockeystate_sub_expire expire_func;
     wsprockeystate_sub_flush flush_func;
     wsprockeystate_sub_destroy destroy_func;
     int instance_len;
     int state_len;
     char * name;
     char * option_str;
     proc_labeloffset_t * labeloffset;
} wsprockeystate_kid_t;

int wsprockeystate_init(int, char *const*, void **, void *, wsprockeystate_kid_t *);
proc_process_t wsprockeystate_input_set(void *, wsdatatype_t *, wslabel_t *,
                                        ws_outlist_t*, int, void *);
int wsprockeystate_destroy(void *);

#ifdef __cplusplus
CPP_CLOSE
#endif // __cplusplus

#endif // _WSPROCKEYSTATE_H
