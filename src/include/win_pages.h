#ifndef __PAGE_SIZE_FIX_H__
#define __PAGE_SIZE_FIX_H__

// MingW32/64 doesn't have _SC_PAGESIZE
#if !defined _SC_PAGESIZE && (defined _WIN32 || defined _WIN64     || defined WINDOWS)

#include <windows.h>
#include <malloc.h>

#define PAGE_SIZE() WinPageSize()
#define ALIGNED_ALLOC WinAlignedAlloc


#ifndef ENOMEM
#define ENOMEM 12
#endif
#ifndef EINVAL
#define EINVAL 22
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline size_t WinPageSize(void) { SYSTEM_INFO si; GetSystemInfo(&si); return si.dwPageSize; }

static inline int WinAlignedAlloc(void** memptr, size_t alignment, size_t size) { return ((*memptr = _aligned_malloc(size, alignment)) != NULL ? 0 : ENOMEM); }


#ifdef __cplusplus
}
#endif

#else
#define PAGE_SIZE() ((size_t)sysconf(_SC_PAGESIZE))
#define ALIGNED_ALLOC posix_memalign
#endif

#endif

