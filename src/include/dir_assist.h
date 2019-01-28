#ifndef __DIR_ASSIST_H__
#define __DIR_ASSIST_H__

/*
 * Cross platform header to assist in directory/file listing
*/

// ret 1 to keep
typedef int (*wsdir_filter_t)(const char*);

// called for each keep
typedef void (*wsdir_callback_t)(
     const char* /*dir*/,
     const char* /*entry*/,
     void* /*opaque*/);

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
// windows
#include <windows.h>

int wsdir_scan(
          const char * path,
          const wsdir_filter_t filter,
          const wsdir_callback_t callback,
          void * opaque)
{
     HANDLE hFind;
     WIN32_FIND_DATA de;
     hFind = FindFirstFile("path", &de);
     if (hFind == INVALID_HANDLE_VALUE) return 0;
     do {
          if (filter(de.cFileName)) {
               callback(path, de.cFileName, opaque);
          }
     }while(FindNextFile(hFind, &de));
     FindClose(hFind);
     return 1;
}

#else
// Posix
#include <sys/stat.h>
#include <dirent.h>

int wsdir_scan(
          const char * path, 
          const wsdir_filter_t filter,
          const wsdir_callback_t callback,
          void * opaque)
{
     DIR * dir;
     dir = opendir(path);
     if (dir==NULL) return 0;
     struct dirent * de;
     while ((de=readdir(dir))!=NULL) {
          if (filter(de->d_name)) {
               callback(path, de->d_name, opaque);
          }
     }
     closedir(dir);
     return 1;
}

#endif // end OS check

#endif

