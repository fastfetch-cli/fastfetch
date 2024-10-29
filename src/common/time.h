#pragma once

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#ifdef _WIN32
    #include <synchapi.h>
    #include <profileapi.h>
    #include <sysinfoapi.h>
#endif

static inline double ffTimeGetTick(void) //In msec
{
    #ifdef _WIN32
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);
        return (double) start.QuadPart * 1000 / (double) frequency.QuadPart;
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

    static char buf[32];
    strftime(buf, __builtin_strlen("0000-00-00T00:00:00") + 1, "%FT%T", tm);
    snprintf(buf + __builtin_strlen("0000-00-00T00:00:00"), __builtin_strlen(".000") + 1, ".%03u", (unsigned) (msec % 1000));
    strftime(buf + __builtin_strlen("0000-00-00T00:00:00.000"), __builtin_strlen("+0000") + 1, "%z", tm);
    return buf;
}

// Not thread-safe
static inline const char* ffTimeToShortStr(uint64_t msec)
{
    if (msec == 0) return "";
    time_t tsec = (time_t) (msec / 1000);

    static char buf[32];
    strftime(buf, sizeof(buf), "%F %T", localtime(&tsec));
    return buf;
}

#ifdef _WIN32
    #pragma GCC diagnostic pop
#endif
