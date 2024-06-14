#ifndef _LOGICAL_CPU_SELECT_H
#define _LOGICAL_CPU_SELECT_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS )
  #include <windows.h>
  #include <sysinfoapi.h> // for proc count
  #include <winternl.h>
#else
  #include <unistd.h>
  #if defined(__FreeBSD__)
    #include <sys/sysctl.h>
    #include <sys/resource.h>
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns an array of OS_index core numbers which are "free" to use.
 *
 * @param minIdle Minimum percentage of CPU time that should be idle to consider a core "idle" (eg. 95.0)
 * @param nRequested Number of logical cores which are being requested.
 * @param cores Array of length nRequested which will be filled in by this function
 * @return 1 on success, 0 on error.
 */
static int getFreeLogicalProcessors(uint32_t nRequested, uint32_t *cores);

static long getLogicalProcessorCount(void);
static int getCoreIdles(uint32_t ncpu, uint64_t *idle, uint64_t *total, int secondCall);

// result struct to pair logical processor index with idle-ness level
typedef struct __cpu_res_struct_ {
    float     level;
    uint32_t  index;
} __cpu_res_struct;

static int getLoadLevels(__cpu_res_struct **levels, uint32_t *ncores);

// to sort by idle-ness level
static int __idleness_cmp(const void* a, const void* b)
{ return ((__cpu_res_struct*)a)->level < ((__cpu_res_struct*)b)->level; }

/**
 * Returns an array of preferred logical processor index numbers to use.
 * NOTE: Indexing starts at 0, even if system uses 1-based indexing externally.
 *
 * @param nRequested Number of logical cores which are being requested.
 * @param indicesOut Array of length nRequested which will be filled in by this function
 * @return 1 on success, 0 on error.
 */
static int getFreeLogicalProcessors(uint32_t nRequested, uint32_t *indicesOut)
{
    assert( indicesOut != NULL );

    __cpu_res_struct * levels = NULL;
    uint32_t count = 0;
    int res = getLoadLevels(&levels, &count);
    if ( !res ) {
        fprintf(stderr, "WARNING: Failed to determine idle-levels of processor cores!\n");
        return 0;
    }

    if ( nRequested > count )
        fprintf(stderr, "WARNING: Requesting more cores than on the system!\n");

    // sort the resulting idle levels by idle-ness first
    qsort(levels,count,sizeof(__cpu_res_struct),__idleness_cmp);

    // map logical processor indices to output results
    int outIndex;
    int resIndex = 0;
    for (outIndex=0; outIndex<nRequested; outIndex++) {
        indicesOut[outIndex] = levels[resIndex].index;
        resIndex = ( resIndex + 1 ) % count;
    }

    // release idleness results
    free(levels);
    return 1;
}




#if !( defined _WIN32 || defined _WIN64 || defined WINDOWS )
// Non-Windows function to get count of total logical processors
static long getLogicalProcessorCount(void)
{
    long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    if ( ncpu < 1 ) {
        fprintf(stderr, "Failed getting number of cpus\n");
        perror("sysconf");
        return 0;
    }
    return ncpu;
}
#else
// Windows-specific function to get count of total logical processors
// (on the current physical CPU group)
static long getLogicalProcessorCount(void)
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD procCount = sysInfo.dwNumberOfProcessors;
    return (long) procCount;
}
#endif

#if defined(__linux)
static int getCoreIdles(uint32_t ncpu, uint64_t *idle, uint64_t *total, int secondCall)
{
    char buffer[1024] = {0};
    uint32_t nRead = 0;
    FILE *fp = fopen("/proc/stat", "r");
    if ( !fp ) {
        perror("fopen");
        return 0;
    }

    while ( (nRead < ncpu) && fgets(buffer, 1024, fp) ) {
        uint32_t cpu = 0;
        int read = sscanf(buffer, "cpu%" PRIu32, &cpu);
        if ( (read == 1) && (cpu < ncpu) ) {
            uint32_t count = 0;
            uint64_t coreTot = 0;
            uint64_t coreIdle = 0;

            // Advance past 'cpu'
            char *buf = &buffer[3];

            errno = 0; // Reset
            unsigned long long val = strtoull(buf, &buf, 10);
            if ( errno != 0 ) {
                perror("strtoull");
                fclose(fp);
                return 0;
            }
            if ( val != cpu ) {
                fprintf(stderr, "Mis-read /proc/stat!\n");
                fclose(fp);
                return 0;
            }
            do {
                errno = 0;
                val = strtoull(buf, &buf, 10);
                if ( errno != 0 ) {
                    perror("strtoull");
                    fclose(fp);
                    return 0;
                }
                coreTot += val;
                if ( count == 3 ) coreIdle = val;
                count++;

            } while ( *buf != '\0' && *buf != '\n' );

            if ( count < 3 ) {
                fprintf(stderr, "Mis-read /proc/stat!\n");
                fclose(fp);
                return 0;
            }

            nRead++;

            if ( secondCall ) {
                idle[cpu] = coreIdle - idle[cpu];
                total[cpu] = coreTot - total[cpu];
            } else {
                idle[cpu] = coreIdle;
                total[cpu] = coreTot;
            }
        }
    }

    fclose(fp);
    return 1;
}
#elif defined(__FreeBSD__)
static int getCoreIdles(uint32_t ncpu, uint64_t *idle, uint64_t *total, int secondCall)
{
     size_t oldlen = 0;
     int ok = sysctlbyname("kern.cp_times", NULL, &oldlen, NULL, 0);
     if ( ok < 0 ) {
          perror("sysctlbyname");
          return 0;
     }
     long *times = (long*)malloc(oldlen);
     if ( NULL == times ) {
          perror("malloc");
          return 0;
     }
     ok = sysctlbyname("kern.cp_times", times, &oldlen, NULL, 0);
     if ( ok < 0 ) {
          perror("sysctlbyname");
          return 0;
     }

     uint32_t nCPUs = (oldlen / sizeof(long)) / CPUSTATES;

     if ( ncpu > nCPUs ) {
          fprintf(stderr, "Only found information on %" PRIu32 " cpus.  Requested %" PRIu32 "\n", nCPUs, ncpu);
          return 0;
     }

     uint32_t c, s;
     for ( c = 0 ; c < nCPUs ; c++ ) {
          uint64_t coreTot = 0;
          for ( s = 0 ; s < CPUSTATES ; s++ ) {
               coreTot += times[(c*CPUSTATES) + s];
          }
          uint64_t coreIdle = times[(c*CPUSTATES) + CP_IDLE];

          if ( secondCall ) {
              idle[c] = coreIdle - idle[c];
              total[c] = coreTot - total[c];
          } else {
              idle[c] = coreIdle;
              total[c] = coreTot;
          }
     }
     free(times);
     return 1;
}
#elif (defined _WIN32 || defined _WIN64 || defined WINDOWS )
static int getCoreIdles(uint32_t ncpu, uint64_t *idle, uint64_t *total, int secondCall)
{
    // NOTE: not currently checking whether core is online/awake

    // Create array of proc-perf structs to query for times
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION * info;
    info = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*) calloc(
        ncpu, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));
    memset(info, 0, ncpu * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));

    // Fill the info query array
    NTSTATUS res = NtQuerySystemInformation(
        SystemProcessorPerformanceInformation,
        info,
        ncpu * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION),
        NULL );
    if ( res != 0 ) {
        fprintf(stderr,"WARNING: Failed to query processor information to detect idle-levels\n");
        return 0;
    }
    // transfer idle-level values to value array for later comparison
    if ( ! secondCall ) {
        uint64_t summ;
        for (int i = 0; i < ncpu; i++) {
            idle[i]  = info[i].IdleTime.QuadPart;
            summ = info[i].UserTime.QuadPart + info[i].KernelTime.QuadPart;
            total[i] = summ;
        }
    } else {
        uint64_t summ;
        for (int i = 0; i < ncpu; i++) {
            idle[i]  = info[i].IdleTime.QuadPart - idle[i];
            summ = info[i].UserTime.QuadPart + info[i].KernelTime.QuadPart;
            total[i] = summ - total[i];
        }
    }
    return 1; // ok
}
#else
#error "Don't know how to get Idle cores on this platform."
#endif


/* Caller must free levels */
static int getLoadLevels(__cpu_res_struct ** levels, uint32_t * count)
{
    long processorCnt = getLogicalProcessorCount();
    if ( processorCnt < 1 ) {
        return 0;
    }

    __cpu_res_struct * level = (__cpu_res_struct*) calloc(
        processorCnt, sizeof(__cpu_res_struct) );
    uint64_t * idle  = (uint64_t*)calloc(processorCnt, sizeof(uint64_t));
    uint64_t * total = (uint64_t*)calloc(processorCnt, sizeof(uint64_t));
    if ( !level || !idle || !total ) {
        perror("calloc");
        if (level) free(level);
        if (idle)  free(idle);
        if (total) free(total);
        return 0;
    }
    
    // Get initial readings
    if ( !getCoreIdles(processorCnt, idle, total, 0) ) {
        free(level);
        free(idle);
        free(total);
        return 0;
    }

    sleep(1);

    // Get updated reading to take the diff
    if ( !getCoreIdles(processorCnt, idle, total, 1) ) {
        free(level);
        free(idle);
        free(total);
        return 0;
    }

    // convert measurement ratios to float percent results
    long c;
    for ( c = 0 ; c < processorCnt ; c++ ) {
        level[c].index = c;
        level[c].level = (100.0f * idle[c]) / total[c];
        //printf("index: %li, level: %f\n",c,level[c].level);
    }

    /* HACK:  We don't like core 0 */
    level[0].level = 75.0f;

    // set result parameters
    *levels = level;
    *count  = (uint32_t)processorCnt;
    // release temp storage
    free(total);
    free(idle);
    // Note: levels not freed
    return 1;
}


#ifdef __cplusplus
}
#endif

#endif
