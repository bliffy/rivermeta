#ifndef _existonly_table_h
#define _existonly_table_h

#include "stringhash9a.h"

#ifdef __cplusplus
extern "C" {
#endif

//generic type
typedef stringhash9a_t eot_t;
typedef stringhash9a_sh_opts_t eot_sh_opts_t;

//aliased, generic function names
typedef void (*eot_sh_opts_alloc)(eot_sh_opts_t **);
typedef void (*eot_sh_opts_free)(eot_sh_opts_t *);
//DEPRECATED
typedef int (*eot_open_table)(eot_t **, const char *);
//CURRENT
typedef int (*eot_open_sht_table)(eot_t **, uint32_t, eot_sh_opts_t *);
typedef eot_t * (*eot_create)(uint32_t, uint32_t);
//DEPRECATED
typedef int (*eot_create_shared)(
     void *, void **, const char *,
     uint32_t, int *, int, void *);
//CURRENT
typedef int (*eot_create_shared_sht)(
     void *, void **, const char *,
     uint32_t, int *, eot_sh_opts_t *);
typedef int (*eot_check)(eot_t *, void *, int);
typedef uint64_t (*eot_drop_cnt)(eot_t *);
typedef int (*eot_set)(eot_t *, void *, int);
typedef int (*eot_delete)(eot_t *, void *, int);
typedef void (*eot_flush)(eot_t *);
typedef void (*eot_destroy)(eot_t *);
typedef eot_t * (*eot_read)(FILE *);
typedef int (*eot_dump)(eot_t *, FILE *);
typedef int (*eot_check_loc)(eot_t *, ws_hashloc_t *);
typedef int (*eot_check_wsdata)(eot_t *, wsdata_t *);
typedef int (*eot_set_loc)(eot_t *, ws_hashloc_t *);
typedef int (*eot_set_wsdata)(eot_t *, wsdata_t *);
typedef int (*eot_delete_loc)(eot_t *, ws_hashloc_t *);
typedef int (*eot_delete_wsdata)(eot_t *, wsdata_t *);

//aliased, generic function equivalencies
eot_sh_opts_alloc existonly_sh_opts_alloc = stringhash9a_sh_opts_alloc;
eot_sh_opts_free existonly_sh_opts_free = stringhash9a_sh_opts_free;
//DEPRECATED
eot_open_table existonly_table_open_table = stringhash9a_open_table;
//CURRENT
eot_open_sht_table existonly_open_sht_table = stringhash9a_open_sht_table;
eot_create existonly_table_create = stringhash9a_create;
//DEPRECATED
eot_create_shared existonly_table_create_shared = stringhash9a_create_shared;
//CURRENT
eot_create_shared_sht existonly_table_create_shared_sht = stringhash9a_create_shared_sht;
eot_check existonly_table_check = stringhash9a_check;
eot_drop_cnt existonly_table_drop_cnt = stringhash9a_drop_cnt;
eot_set existonly_table_set = stringhash9a_set;
eot_delete existonly_table_delete = stringhash9a_delete;
eot_flush existonly_table_flush = stringhash9a_flush;
eot_destroy existonly_table_destroy = stringhash9a_destroy;
eot_read existonly_table_read = stringhash9a_read;
eot_dump existonly_table_dump = stringhash9a_dump;
eot_check_loc existonly_table_check_loc = stringhash9a_check_loc;
eot_check_wsdata existonly_table_check_wsdata = stringhash9a_check_wsdata;
eot_set_loc existonly_table_set_loc = stringhash9a_set_loc;
eot_set_wsdata existonly_table_set_wsdata = stringhash9a_set_wsdata;
eot_delete_loc existonly_table_delete_loc = stringhash9a_delete_loc;
eot_delete_wsdata existonly_table_delete_wsdata = stringhash9a_delete_wsdata;

#ifdef __cplusplus
}
#endif

#endif // _existonly_table_h

