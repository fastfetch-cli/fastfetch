#pragma once

#ifndef FF_INCLUDED_common_time
#define FF_INCLUDED_common_time

#include <stdint.h>
#if defined(_WIN32) || defined(__MSYS__)
    #include <synchapi.h>
    #include <sysinfoapi.h>
#else
    #include <sys/time.h>
    #include <time.h>
#endif

static inline uint64_t ffTimeGetTick() //In msec
{
    #if defined(_WIN32) || defined(__MSYS__)
        return GetTickCount64();
    #else
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        return (uint64_t)((timeNow.tv_sec * 1000) + (timeNow.tv_usec / 1000));
    #endif
}

static inline void ffTimeSleep(uint32_t msec)
{
    #if defined(_WIN32) || defined(__MSYS__)
        SleepEx(msec, TRUE);
    #else
        nanosleep(&(struct timespec){ msec / 1000, (msec % 1000) * 1000000 }, NULL);
    #endif
}

#endif
