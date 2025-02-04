#ifndef _WSPERF_H
#define _WSPERF_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include "shared/getrank.h"
#include "insertion_sort.h"
#include "mimo.h"

#ifdef __cplusplus
extern "C" {
#endif

// define that controls wsperf sampling rate
#define WSPERF_INTERVAL (100)

// Macros for WSPERF functions
#ifdef WS_PERF
#define WSPERF_BASE_TIME() extern uint64_t * wsbase; wsbase[nrank]=get_cycle_count();
#define WSPERF_NRANK() const int nrank = GETRANK();
#define WSPERF_LOCAL_INIT() uint64_t mon_base = 0;
#define WSPERF_TIME0(uid) numcalls[uid]++; if (!(numcalls[uid]%WSPERF_INTERVAL)) mon_base=get_cycle_count();
#define WSPERF_PROC_COUNT(uid) (kidcount[nrank][uid])++;
#define WSPERF_FLUSH_COUNT(uid) (flushcount[nrank][uid])++;
#define WSPERF_TIME(uid) if (!(numcalls[uid]%WSPERF_INTERVAL)) (kidcycle[nrank][uid]) += (get_cycle_count()-mon_base);
#define GET_CYCLE_COUNT() get_cycle_count()
#define INIT_WSPERF() init_wsperf()
#define INIT_WSPERF_KID_NAME(uid,name) init_wsperf_kid_name(uid,name)
#define FREE_WSPERF() free_wsperf()
#define PRINT_WSPERF(mimo) print_wsperf(mimo)

#else
#define WSPERF_BASE_TIME()
#define WSPERF_NRANK()
#define WSPERF_LOCAL_INIT()
#define WSPERF_TIME0(uid)
#define WSPERF_PROC_COUNT(uid) 
#define WSPERF_FLUSH_COUNT(uid) 
#define WSPERF_TIME(uid) 
#define GET_CYCLE_COUNT() 0
#define INIT_WSPERF() 1
#define INIT_WSPERF_KID_NAME(uid,name) 1
#define FREE_WSPERF()
#define PRINT_WSPERF(mimo) 1
#define UPDATE_WSPERF(mimo) 1
#endif // WS_PERF

extern uint32_t max_kids; // same for all threads

extern uint64_t ** kidcycle;
extern uint64_t ** kidcount, ** flushcount, * numcalls;
extern char ** kidname;

#define MICROTICKS 1000000
#define NANOTICKS 1000000000
#define K100TICKS 100000

extern uint64_t hertz; // same for all threads

// Prototypes
uint64_t get_cycle_count(void);
int init_wsperf(void);
int init_wsperf_kid_name(uint32_t, char *);
void free_wsperf(void);
int print_wsperf(mimo_t *);
void init_mg_connection(mimo_t *,int);

#ifdef __cplusplus
}
#endif

#endif // _WSPERF_H
