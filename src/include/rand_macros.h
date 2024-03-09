#ifndef __RANDOM_MACROS_H__
#define __RANDOM_MACROS_H__

//#define _POSIX_THREAD_SAFE_FUNCTIONS  // for rand_r() in mingw
#ifndef _CRT_RAND_S
  #define _CRT_RAND_S  // for rand_s() on windows
#endif
#include <stdlib.h>

// if - is windows
#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)

// Windows has:
// errno_t rand_s(unsigned int* randomValue);
int win_rand_r(unsigned int* seed) { (void)rand_s(seed); return *seed; }

// TODO replace with cryptographically sound alternative
void win_srand48(long seed) { srand((unsigned int)seed); }
long win_mrand48(void) { unsigned int seed = rand(); (void)rand_s(&seed); return seed; }

#define RAND_R(s) win_rand_r(s)
#define SRAND48(s) win_srand48(s)
#define MRAND48 win_mrand48

#else // - is linux

#define RAND_R(s) rand_r(s)
#define SRAND48(s) srand48(s)
#define MRAND48 mrand48

#endif // - end linux

#endif
