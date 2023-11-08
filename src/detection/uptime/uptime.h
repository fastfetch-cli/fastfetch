#pragma once

#include "fastfetch.h"

typedef struct FFUptimeResult
{
    uint64_t bootTime;
    uint64_t uptime;
} FFUptimeResult;

const char* ffDetectUptime(FFUptimeResult* result);
