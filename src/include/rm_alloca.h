//
// Util to include appropriate target-specific header for alloca().
//

#ifndef __RM_ALLOCA_UTIL_H__
#define __RM_ALLOCA_UTIL_H__

#if !defined(alloca)
  #if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
    #include <malloc.h>
    #if !defined(alloca)
      #define alloca _alloca
    #endif
  #elif defined(__GLIBC__) || defined(__sun)
    #include <alloca.h>
  #else
    #include <stdlib.h>
  #endif
#endif

#endif
