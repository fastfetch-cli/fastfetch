#pragma once

#ifndef FF_INCLUDED_detection_cpuusage_cpuusage
#define FF_INCLUDED_detection_cpuusage_cpuusage

#include "fastfetch.h"

typedef struct FFCpuUsageInfo {
    uint64_t inUseAll;
    uint64_t totalAll;
} FFCpuUsageInfo;
const char* ffGetCpuUsageInfo(FFlist* cpuTimes);

const char* ffGetCpuUsageResult(FFlist* result); // list of double

#endif
