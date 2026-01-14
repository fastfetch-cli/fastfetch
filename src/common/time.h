#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
    #include <ntdef.h>
    #include <ntstatus.h>
    #include <profileapi.h>
    #include <sysinfoapi.h>

    NTSYSCALLAPI
    NTSTATUS
    NTAPI
    NtDelayExecution(
        _In_ BOOLEAN Alertable,
        _In_ PLARGE_INTEGER DelayInterval);
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

static inline uint64_t ffTimeGetNow(void)
{
    #ifdef _WIN32
        uint64_t timeNow;
        GetSystemTimeAsFileTime((FILETIME*) &timeNow);
        return (timeNow - 116444736000000000ull) / 10000ull;
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
        return NtDelayExecution(TRUE, &interval) == STATUS_SUCCESS;
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
