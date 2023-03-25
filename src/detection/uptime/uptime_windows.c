#include "uptime.h"

#include <sysinfoapi.h>

uint64_t ffDetectUptime()
{
    return GetTickCount64() / 1000;
}
