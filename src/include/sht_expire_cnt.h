#ifndef _SHT_EXPIRE_CNT_H
#define _SHT_EXPIRE_CNT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "waterslide.h"
#include "error_print.h"

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes
uint64_t stringhash5_expire_cnt(void*);
uint64_t stringhash9a_expire_cnt(void*);

#ifdef __cplusplus
}
#endif

#endif // _SHT_EXPIRE_CNT_H
