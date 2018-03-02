#ifndef _TOOL_PRINT_H
#define _TOOL_PRINT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// macros for printing help...
#if defined(__GNUC__) && __GNUC__ < 3
#ifdef PROC_NAME
#define tool_print(str, ...) fprintf(stderr, "%s " str "\n", PROC_NAME, ##__VA_ARGS__)
#define tool_print(str) fprintf(stderr, "%s " str "\n", PROC_NAME)
#elif defined(TOOL_NAME)
#define tool_print(str, ...) fprintf(stderr, "%s " str "\n", TOOL_NAME, ##__VA_ARGS__)
#define tool_print(str) fprintf(stderr, "%s " str "\n", TOOL_NAME)
#else
#define tool_print(str, ...) fprintf(stderr, str "\n", ##__VA_ARGS__)
#define tool_print(str) fprintf(stderr, str "\n")
#endif // PROC_NAME

#else
#ifdef PROC_NAME
#define tool_print(str, ...) fprintf(stderr, "%s " str "\n", PROC_NAME, ##__VA_ARGS__)
#elif defined(TOOL_NAME)
#define tool_print(str, ...) fprintf(stderr, "%s " str "\n", TOOL_NAME, ##__VA_ARGS__)
#else
#define tool_print(str, ...) fprintf(stderr, str "\n", ##__VA_ARGS__)
#endif // PROC_NAME

#endif // __GNUC__ && __GNUC__ < 3

#ifdef __cplusplus
}
#endif

#endif

