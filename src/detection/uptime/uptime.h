#pragma once

#ifndef FF_INCLUDED_detection_uptime_uptime
#define FF_INCLUDED_detection_uptime_uptime

#include "fastfetch.h"

typedef struct FFUptimeResult
{
    uint64_t bootTime;
    uint64_t uptime;
} FFUptimeResult;

const char* ffDetectUptime(FFUptimeResult* result);

#endif
