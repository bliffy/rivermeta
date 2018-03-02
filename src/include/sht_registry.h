#ifndef _SHT_REGISTRY_H
#define _SHT_REGISTRY_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "error_print.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t max_kids; // same for all threads
extern uint32_t sht_perf;

typedef struct _sht_registry_t {
     char*    sh_type;
     char*    sh_kidname;
     char*    sh_label;
     void*    sht;
     uint64_t size;
     uint64_t expire_cnt;
     uint32_t hash_seed;
} sht_registry_t;

extern sht_registry_t* sh_registry;
extern sht_registry_t** loc_registry;

// Prototypes
int init_sht_registry(void);
void free_sht_registry(void);
void save_proc_name(const char*);
int move_sht_to_local_registry(void*, int*);
int verify_shared_tables(void);
int enroll_shared_in_sht_registry(
          void*, const char*, const char*,
          const uint64_t, const uint32_t);
int enroll_in_sht_registry(
          void*, const char*,
          const uint64_t, const uint32_t);
void get_sht_expire_cnt(void);
void get_sht_shared_expire_cnt(void);
void print_sht_registry(const uint32_t, const uint32_t);

#ifdef __cplusplus
}
#endif

#endif // _SHT_REGISTRY_H
