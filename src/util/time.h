#pragma once

#include <stdint.h>
#include <time.h>
#ifdef _WIN32
    #include <synchapi.h>
    #include <profileapi.h>
    #include <sysinfoapi.h>
#elif defined(__HAIKU__)
    #include <OS.h>
#endif

#include "util/arrayUtils.h"

static inline double ffTimeGetTick(void) //In msec
{
    #ifdef _WIN32
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);
        return (double) start.QuadPart * 1000 / (double) frequency.QuadPart;
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

static inline void ffTimeSleep(uint32_t msec)
{
    #ifdef _WIN32
        SleepEx(msec, TRUE);
    #else
        nanosleep(&(struct timespec){ msec / 1000, (long) (msec % 1000) * 1000000 }, NULL);
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
