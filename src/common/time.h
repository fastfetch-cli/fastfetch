#pragma once

#include <stdint.h>
#include <time.h>
#include <stdio.h>
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

#ifdef _WIN32
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat"
#endif

// Not thread-safe
static inline const char* ffTimeToFullStr(uint64_t msec)
{
    if (msec == 0) return "";
    time_t tsec = (time_t) (msec / 1000);
    const struct tm* tm = localtime(&tsec);

    extern char ffTimeInternalBuffer[64];
    uint32_t len = 0;
    len += (uint32_t) strftime(ffTimeInternalBuffer, ARRAY_SIZE(ffTimeInternalBuffer) - len, "%FT%T", tm);
    len += (uint32_t) snprintf(ffTimeInternalBuffer + len, ARRAY_SIZE(ffTimeInternalBuffer) - len, ".%03u", (unsigned) (msec % 1000));
    len += (uint32_t) strftime(ffTimeInternalBuffer + len, ARRAY_SIZE(ffTimeInternalBuffer) - len, "%z", tm);
    return ffTimeInternalBuffer;
}

// Not thread-safe
static inline const char* ffTimeToShortStr(uint64_t msec)
{
    if (msec == 0) return "";
    time_t tsec = (time_t) (msec / 1000);

    extern char ffTimeInternalBuffer[64];
    strftime(ffTimeInternalBuffer, ARRAY_SIZE(ffTimeInternalBuffer), "%F %T", localtime(&tsec));
    return ffTimeInternalBuffer;
}

// Not thread-safe
static inline const char* ffTimeToTimeStr(uint64_t msec)
{
    if (msec == 0) return "";
    time_t tsec = (time_t) (msec / 1000);

    extern char ffTimeInternalBuffer[64];
    uint32_t len = (uint32_t) strftime(ffTimeInternalBuffer, ARRAY_SIZE(ffTimeInternalBuffer), "%T", localtime(&tsec));
    sprintf(ffTimeInternalBuffer + len, ".%03u", (unsigned) (msec % 1000));
    return ffTimeInternalBuffer;
}

#ifdef _WIN32
    #pragma GCC diagnostic pop
#endif

typedef struct FFTimeGetAgeResult
{
    uint32_t years;
    uint32_t daysOfYear;
    double yearsFraction;
} FFTimeGetAgeResult;

static inline FFTimeGetAgeResult ffTimeGetAge(uint64_t birthMs, uint64_t nowMs)
{
    FFTimeGetAgeResult result = {};
    if (__builtin_expect(birthMs == 0 || nowMs < birthMs, 0))
        return result;

    time_t birth_s = (time_t) (birthMs / 1000);
    struct tm birth_tm;
    #ifdef _WIN32
    localtime_s(&birth_tm, &birth_s);
    #else
    localtime_r(&birth_s, &birth_tm);
    #endif

    time_t now_s = (time_t) (nowMs / 1000);
    struct tm now_tm;
    #ifdef _WIN32
    localtime_s(&now_tm, &now_s);
    #else
    localtime_r(&now_s, &now_tm);
    #endif

    result.years = (uint32_t) (now_tm.tm_year - birth_tm.tm_year);
    if (now_tm.tm_yday < birth_tm.tm_yday)
        result.years--;

    birth_tm.tm_year += (int) result.years;
    birth_s = mktime(&birth_tm);
    uint32_t diff_s = (uint32_t) (now_s - birth_s);
    result.daysOfYear = diff_s / (24 * 60 * 60);

    birth_tm.tm_year += 1;
    result.yearsFraction = (double) diff_s / (double) (mktime(&birth_tm) - birth_s) + result.years;

    return result;
}
