#pragma once

#ifndef FF_INCLUDED_common_time
#define FF_INCLUDED_common_time

#include <stdint.h>
#ifdef _WIN32
    #include <synchapi.h>
    #include <profileapi.h>
    #include <sysinfoapi.h>
#else
    #include <time.h>
#endif

static inline uint64_t ffTimeGetTick() //In msec
{
    #ifdef _WIN32
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        LARGE_INTEGER start;
        QueryPerformanceCounter(&start);
        return (uint64_t)(start.QuadPart * 1000 / frequency.QuadPart);
    #else
        struct timespec timeNow;
        clock_gettime(CLOCK_MONOTONIC, &timeNow);
        return (uint64_t)((timeNow.tv_sec * 1000) + (timeNow.tv_nsec / 1000000));
    #endif
}

static inline uint64_t ffTimeGetNow()
{
    #ifdef _WIN32
        uint64_t timeNow;
        GetSystemTimeAsFileTime((FILETIME*) &timeNow);
        return (timeNow - 116444736000000000ull) / 10000ull;
    #else
        struct timespec timeNow;
        clock_gettime(CLOCK_REALTIME, &timeNow);
        return (uint64_t)((timeNow.tv_sec * 1000) + (timeNow.tv_nsec / 1000000));
    #endif
}

static inline void ffTimeSleep(uint32_t msec)
{
    #ifdef _WIN32
        SleepEx(msec, TRUE);
    #else
        nanosleep(&(struct timespec){ msec / 1000, (msec % 1000) * 1000000 }, NULL);
    #endif
}

#endif
