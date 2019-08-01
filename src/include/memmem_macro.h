#ifndef __MEMMEM_MACRO_H__
#define __MEMMEM_MACRO_H__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
// -- WINDOWS --

static inline void * win_memmem(
        const void * haystack,
        size_t hlen,
        const void * needle,
        size_t nlen) {
    if (haystack == NULL) return NULL;
    if (hlen < nlen) return NULL;
    if (needle == NULL) return NULL;
    if (nlen == 0) return NULL;

    for (const char * h = haystack;
         hlen >= nlen;
         ++h, --hlen)
    {
        if (!memcmp(h, needle, nlen)) {
            return (void*)h;
        }
    }
    return NULL;
}

#define MEMMEM(a,b,c,d) win_memmem(a,b,c,d)

#else // -- posix --

#define MEMMEM(a,b,c,d) memmem(a,b,c,d)

#endif // -- end OS check --

#ifdef __cplusplus
}
#endif

#endif
