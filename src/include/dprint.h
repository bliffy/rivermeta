#ifndef _DPRINT_H
#define _DPRINT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Best to define this in specific source files of interest!
//#define DEBUG

// macros for printing help...
#if defined(__GNUC__) && __GNUC__ < 3
#ifdef DEBUG
#define  dprint(str, ...) fprintf(stderr,str "\n", ##__VA_ARGS__)
#define  dprint(str) fprintf(stderr,str "\n")
#else
#define dprint(str, ...)
#endif // DEBUG

#else
#ifdef DEBUG
#define  dprint(str, ...) fprintf(stderr,str "\n", ##__VA_ARGS__)
#else
#define dprint(str, ...)
#endif
#endif // DEBUG

#ifdef __cplusplus
}
#endif

#endif // _DPRINT_H
