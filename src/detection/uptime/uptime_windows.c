#include "uptime.h"

#include <sysinfoapi.h>

const char* ffDetectUptime(uint64_t* result)
{
    *result = GetTickCount64() / 1000;
    return NULL;
}
