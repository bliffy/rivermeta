#ifndef __WIN_EXTRA_STUFF_H__
#define __WIN_EXTRA_STUFF_H__

#include <stddef.h>

// Defining more things missing on windows

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)

char * _strndup(const char * src, size_t n) {
    const char * p = memchr(src, '\0', n);
    if (p != NULL) {
        size_t e = (p-src);
        if (e>n) e = n;
        char * r = malloc(e+1);
        if (!r) return NULL;
        n = p - src;
        memcpy(r, src, e);
        r[e] = '\0';
        return r;
    } else {
        char * r = malloc(n+1);
        if (!r) return NULL;
        memcpy(r, src, n);
        r[n] = '\0';
        return r;
    }
}

char * _strsep(char ** stringp, const char * sep) {
    if (!stringp || !sep || !*stringp) return NULL;
    char * begin = *stringp;
    const size_t l = strlen(sep);
    if (l==0) return NULL;
    for (char * e=begin; *e!=0; e++) {
        for (size_t s=0; s<l; s++) {
            if (sep[s]==*e) {
                *e = '\0';
                *stringp = e+1;
                return begin;
            }
        }
    }
    *stringp = NULL;
    return begin;
}

#else

#include <string.h>
#define _strndup(s,n) strndup(s,n)
#define _strsep(str,sep) strsep(str,sep)

#endif

#endif
