#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
    #include <ntstatus.h>
    #include "common/windows/nt.h"
    #include <profileapi.h>
#elif defined(__HAIKU__)
    #include <OS.h>
#endif

#include "common/arrayUtils.h"

static inline double ffTimeGetTick(void) //In msec
{
    #ifdef _WIN32
        extern double ffQpcMultiplier;
        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);
        return (double) start.QuadPart * ffQpcMultiplier;
    #elif defined(__HAIKU__)
        return (double) system_time() / 1000.;
    #else
        struct timespec timeNow;
        clock_gettime(CLOCK_MONOTONIC, &timeNow);
        return (double) timeNow.tv_sec * 1000. + (double) timeNow.tv_nsec / 1000000.;
    #endif
}

#if _WIN32
static inline uint64_t ffFileTimeToUnixMs(uint64_t value)
{
    if (__builtin_expect(__builtin_usubll_overflow(value, 116444736000000000ull, &value), false))
        return 0;
    return value / 10000ull;
}
#endif

static inline uint64_t ffTimeGetNow(void)
{
    #ifdef _WIN32
        uint64_t timeNow = ffKSystemTimeToUInt64(&SharedUserData->SystemTime);
        return ffFileTimeToUnixMs((uint64_t) timeNow);
    #elif defined(__HAIKU__)
        return (uint64_t) real_time_clock_usecs() / 1000u;
    #else
        struct timespec timeNow;
        clock_gettime(CLOCK_REALTIME, &timeNow);
        return (uint64_t)(((uint64_t) timeNow.tv_sec * 1000u) + ((uint64_t) timeNow.tv_nsec / 1000000u));
    #endif
}

// Returns true if not interrupted
static inline bool ffTimeSleep(uint32_t msec)
{
    #ifdef _WIN32
        LARGE_INTEGER interval;
        interval.QuadPart = -(int64_t) msec * 10000; // Relative time in 100-nanosecond intervals
        return NT_SUCCESS(NtDelayExecution(TRUE, &interval));
    #else
        return nanosleep(&(struct timespec){ msec / 1000, (long) (msec % 1000) * 1000000 }, NULL) == 0;
    #endif
}

// Not thread-safe
const char* ffTimeToFullStr(uint64_t msec);

// Not thread-safe
const char* ffTimeToShortStr(uint64_t msec);

// Not thread-safe
const char* ffTimeToTimeStr(uint64_t msec);

typedef struct FFTimeGetAgeResult
{
    uint32_t years;
    uint32_t daysOfYear;
    double yearsFraction;
} FFTimeGetAgeResult;

FFTimeGetAgeResult ffTimeGetAge(uint64_t birthMs, uint64_t nowMs);
