#ifndef _WSDT_BUNDLE_H
#define _WSDT_BUNDLE_H

#include <stdint.h>

#include "waterslide.h" // wsdata_t

#define WSDT_BUNDLE_STR "BUNDLE_TYPE"

#define WSDT_BUNDLE_MAX 1023

typedef struct _wsdt_bundle_t {
     int len;
     wsdata_t * wsd[WSDT_BUNDLE_MAX];
} wsdt_bundle_t;

#endif
