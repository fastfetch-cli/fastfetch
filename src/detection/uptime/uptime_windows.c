#include "uptime.h"

#define WIN32_LEAN_AND_MEAN
#include <sysinfoapi.h>

uint64_t ffDetectUptime()
{
    return GetTickCount64() / 1000;
}
