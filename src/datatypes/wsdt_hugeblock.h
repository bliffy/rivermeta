#ifndef _WSDT_HUGEBLOCK_H
#define _WSDT_HUGEBLOCK_H

#include <stdint.h>

// the following is not a limit, just a big size
#define WSDT_HUGEBLOCK_LEN (1<<31)

#define WSDT_HUGEBLOCK_STR "HUGEBLOCK_TYPE"

typedef struct _wsdt_hugeblock_t {
     uint64_t len;      // this field is more like a capacity
                        // field; we use it to reserve room 
                        // for memory allocated
     uint64_t actual_len; // used to store actual length of 
                          // actual content (bytes) in our
                          // huge block
     uint32_t num_items; // can ignore when buf is not holding items
     uint32_t seqno;    // sequence number of this hugeblock
     uint16_t linklen;  // if used, this should specify link-layer len for items in hugeblock
     int linktype;      // if used, this should specify link-layer type for items in hugeblock
     char * buf;
} wsdt_hugeblock_t;

#endif

