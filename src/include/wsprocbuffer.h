#ifndef _WSPROCBUFFER_H
#define _WSPROCBUFFER_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
//#include <netinet/in.h>
#include <time.h>
#include <assert.h>
#include "waterslide.h"
#include "waterslidedata.h"
#include "datatypes/wsdt_tuple.h"
#include "wstypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*wsprocbuffer_sub_init)(void *, void *);
typedef int (*wsprocbuffer_sub_option)(
     void *,
     void *,
     int,
     const char *);
typedef int (*wsprocbuffer_sub_decode)(
     void *,
     wsdata_t *,
     wsdata_t *,
     const char *,
     size_t);
typedef int (*wsprocbuffer_sub_element)(
     void *,
     wsdata_t *,
     wsdata_t *);
typedef int (*wsprocbuffer_sub_destroy)(void *);

typedef struct _wsprocbuffer_kid_t {
     wsprocbuffer_sub_init init_func;
     wsprocbuffer_sub_option option_func;
     wsprocbuffer_sub_decode decode_func;
     wsprocbuffer_sub_element element_func;
     wsprocbuffer_sub_destroy destroy_func;
     int pass_not_found; // flag to indicate if tuple should
                         // be passed if LABEL does not exist
                         // in that tuple
     int instance_len;
     char * name;
     char * option_str;
     proc_labeloffset_t * labeloffset;
} wsprocbuffer_kid_t;

int wsprocbuffer_init(
     int,
     char * const *,
     void **,
     void *,
     wsprocbuffer_kid_t *);
proc_process_t wsprocbuffer_input_set(
     void *,
     wsdatatype_t *,
     wslabel_t *,
     ws_outlist_t *,
     int,
     void *);
int wsprocbuffer_destroy(void *);

#ifdef __cplusplus
}
#endif

#endif // _WSPROCBUFFER_H

