#include "uptime.h"

#include <sysinfoapi.h>

static inline uint64_t to_ms(uint64_t ret)
{
    ret -= 116444736000000000ull;
    return ret / 10000ull;
}

const char* ffDetectUptime(FFUptimeResult* result)
{
    result->uptime = GetTickCount64();
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);
    result->bootTime = to_ms(*(uint64_t*) &fileTime) - result->uptime;
    return NULL;
}
