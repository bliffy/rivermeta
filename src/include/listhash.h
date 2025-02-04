//LISTHASH

// purpose: non-expiring hash table library that is designed for fast lookup
// and balanced insertion..
// uses link lists
// currently cannot remove data except for flushing the entire table..

//important functions are
//  lh = listhash_create( max_record, data_alloc_size);
//  vdata = listhash_find(lh, key_str, keylen);
//  vdata = listhash_find_attach(lh, key_str, keylen);
//  vdata = listhash_find_attach_reference(lh, key_str, keylen, void *);
//  listhash_flush(lh);

#ifndef _LISTHASH_H
#define _LISTHASH_H

#include <stdint.h>
#include "error_print.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LH_HASHSEED 0x11F00FED

#define LH_DIGEST_DEPTH 7

struct _listhash_data_t;
typedef struct _listhash_data_t listhash_data_t;

struct _listhash_digest_t;
typedef struct _listhash_digest_t listhash_digest_t;

struct _listhash_row_t;
typedef struct _listhash_row_t listhash_row_t;

typedef struct _listhash_t {
     listhash_row_t*  table;    
     uint32_t         records; // current count
     uint32_t         max_index; // used as an estimate.
     uint32_t         mask; // mask for calculating index
     uint32_t         bits; // mask for setting index
     // size of auxillary data for each flow rec
     uint32_t         data_alloc;
} listhash_t;

listhash_t* listhash_create(int max_records, int data_alloc);
//find records... then use them
void* listhash_find(listhash_t* lh, const char* key, int keylen);

void* listhash_find_attach(listhash_t* lh, const char* key, int keylen);

void* listhash_find_attach_reference(
          listhash_t* lh,
          const char* key,
          int keylen,
          void* ref);

//completely clean out table, but keep table intact
void listhash_flush(listhash_t* lh);
//destroy entire table... free all memory
void listhash_destroy(listhash_t* lh);

typedef void (*listhash_func_t)(void* /*data*/, void* /*userdata*/);

//touch all non-reference records with user defined func
void listhash_scour(listhash_t*, listhash_func_t, void*);

#ifdef __cplusplus
}
#endif

#endif // _LISTHASH_H
