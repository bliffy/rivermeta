#ifndef __WS_LABEL_H__
#define __WS_LABEL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _wslabel_t {
     uint8_t registered;
     uint8_t search;
     uint16_t index_id;
     uint64_t hash;
     char * name;  //null terminated string
} wslabel_t; // stored in mimo->datalists->label_table

#define WSMAX_LABEL_SET 128
typedef struct _wslabel_set_t {
     int len;
     wslabel_t * labels[WSMAX_LABEL_SET];
     int id[WSMAX_LABEL_SET];
} wslabel_set_t;

#define WSLABEL_NEST_SUBSET 32
typedef struct _wslabel_nested_set_t {
     wslabel_set_t lset[WSLABEL_NEST_SUBSET];
     int subsets;
     int cnt;
} wslabel_nested_set_t;

typedef struct _wslabel_set_ext_t {
     int len;
     wslabel_t * labels[WSMAX_LABEL_SET];
     int uid[WSMAX_LABEL_SET];
     int nid[WSMAX_LABEL_SET];
} wslabel_set_ext_t;

typedef struct _wslabel_nested_set_ext_t {
     wslabel_set_ext_t lset[WSLABEL_NEST_SUBSET];
     int subsets;
     int cnt;
} wslabel_nested_set_ext_t;

/*
typedef struct _ws_hashloc_t {
     void * offset;  //offset into data
     int len;     //length of data to hash
} ws_hashloc_t;
*/

wslabel_t * wsregister_label(void *, const char * );
wslabel_t * wsregister_label_len(void *, const char *, int);
wslabel_t * wsregister_label_wprefix(
     void *,
     const char *,
     const char *);
int wsregister_label_alias(
     void *,
     wslabel_t *,
     const char *);

wslabel_t * wssearch_label(void *, const char * );
wslabel_t * wssearch_label_len(void *, const char *, int);

wslabel_t * wslabel_find_byhash(void *, uint64_t);
int wslabel_match(
     void *,
     const wslabel_t *,
     const char *);

int wslabel_nested_search_build(
     void *,
     wslabel_nested_set_t *,
     const char *);
int wslabel_nested_search_build_ext(
     void *,
     wslabel_nested_set_ext_t *,
     const char *,
     int);
int wslabel_set_add(
     void *,
     wslabel_set_t *,
     const char *);
int wslabel_set_add_noindex(
     void *,
     wslabel_set_t *,
     const char *);
int wslabel_set_add_id(
     void *,
     wslabel_set_t *,
     const char *,
     int id);
int wslabel_set_add_id_len(
     void *,
     wslabel_set_t *,
     const char *,
     int,
     int);

#ifdef __cplusplus
}
#endif

#endif

