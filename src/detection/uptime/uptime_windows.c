#include "uptime.h"

#define WIN32_LEAN_AND_MEAN
#include <sysinfoapi.h>

uint64_t ffDetectUptime(const FFinstance* instance)
{
    FF_UNUSED(instance)
    return GetTickCount64() / 1000;
}
