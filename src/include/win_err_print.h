#ifndef __WIN_ERROR_PRINT_H__
#define __WIN_ERROR_PRINT_H__

#include <windows.h>
#include "error_print.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void error_print_win_error(void)
{
    DWORD err = GetLastError();
    char msg[2048];
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) msg,
        2047, NULL );
    error_print("%s",msg);
}

#ifdef __cplusplus
}
#endif

#endif
