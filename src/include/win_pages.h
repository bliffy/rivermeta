#ifndef __PAGE_SIZE_FIX_H__
#define __PAGE_SIZE_FIX_H__

#include <windows.h>
#include <malloc.h>

#ifndef ENOMEM
#define ENOMEM 12
#endif
#ifndef EINVAL
#define EINVAL 22
#endif

inline size_t WinPageSize(void) { SYSTEM_INFO si; GetSystemInfo(&si); return si.dwPageSize; }
inline int WinAlignedAlloc(void** memptr, size_t alignment, size_t size) { return ((*memptr = _aligned_malloc(size, alignment)) != NULL ? 0 : ENOMEM); }

#endif