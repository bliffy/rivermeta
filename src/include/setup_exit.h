#ifndef _SETUP_EXIT_H
#define _SETUP_EXIT_H

#include "mimo.h"

#ifdef __cplusplus 
extern "C" {
#endif

// Prototypes
extern void clean_exit(int);
extern void setup_exit_signals(void);
extern int ws_cleanup(mimo_t* mimo);

#ifdef __cplusplus
}
#endif

#endif // _SETUP_EXIT_H
