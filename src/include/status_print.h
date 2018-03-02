// status_print.h
// (extracted from waterslide.h)

#ifndef _STATUS_PRINT_H
#define _STATUS_PRINT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// macros for printing help...
#if defined(__GNUC__) && __GNUC__ < 3
#define status_print(str, ...) fprintf(stderr, str "\n", ##__VA_ARGS__)
#define status_print(str) fprintf(stderr, str "\n")
#else
#define status_print(str, ...) fprintf(stderr, str "\n", ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif // _STATUS_PRINT_H
