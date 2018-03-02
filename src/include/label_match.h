#ifndef _LABEL_MATCH_H
#define _LABEL_MATCH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "waterslide.h"
#include "ahocorasick.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LABEL_MATCH_MAX_LABELS 2000

// instance data structure
typedef struct _label_match_t {
     ahoc_t* ac_struct;
     wslabel_t* label[LABEL_MATCH_MAX_LABELS];
     int label_cnt;
     void* type_table;
} label_match_t;

//initialize variables
label_match_t* label_match_create(void* type_table);
int label_match_loadfile(label_match_t*, char*);
ahoc_t * label_match_finalize(label_match_t*);
wslabel_t* label_match_get_label(label_match_t*, int);
int label_match_make_label(label_match_t* lm, const char* str);
void label_match_destroy(label_match_t*);

#ifdef __cplusplus
}
#endif

#endif // _LABEL_MATCH_H
