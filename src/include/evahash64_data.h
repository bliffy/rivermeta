#ifndef _EVAHASH64_DATA_H
#define _EVAHASH64_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "evahash64.h"
#include "waterslide.h"
#include "waterslidedata.h"

static inline uint64_t evahash64_data(
          wsdata_t* wsd,
          uint32_t seed)
{
     uint64_t res = 0;
     if ( wsd ) {
          ws_hashloc_t* hashloc = wsd->dtype->hash_func(wsd);
          if ( hashloc && hashloc->offset ) {
               res = evahash64((uint8_t*)hashloc->offset, hashloc->len, seed);
          }
     }
     return res;
}

#ifdef __cplusplus
}
#endif

#endif // _EVAHASH64_DATA_H
