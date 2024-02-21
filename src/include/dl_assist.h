#ifndef __DL_ASSIST_H__
#define __DL_ASSIST_H__

/*
 * Cross platform header to assist in DLL/SO runtime loading
*/

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
// windows
#include <windows.h>

#define DLHANDLE HINSTANCE
#define DLOPEN(p) LoadLibrary(p)
#define DLSYM(h,N) GetProcAddress(h,N)
#define DLCLOSE(h) FreeLibrary(h)
// TODO: needs testing
void DLERROR(char * buf, size_t max) {
     FormatMessage(
          FORMAT_MESSAGE_FROM_SYSTEM
               | FORMAT_MESSAGE_ARGUMENT_ARRAY
               | FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          GetLastError(),
          0,
          (LPSTR) buf,
          max,
          NULL);
}

#else
// Posix
#include <dlfcn.h>

#define DLHANDLE void*
#define DLOPEN(p) dlopen(p,RTLD_NOW)
#define DLSYM(h,N) dlsym(h,N)
/* ( TODO: should check dlerror() */
#define DLCLOSE(h) dlclose(h)
void DLERROR(char * buf, size_t max) {
     const char * err = dlerror();
     strncpy(buf,err,max);
}

#endif // end OS check

#endif

