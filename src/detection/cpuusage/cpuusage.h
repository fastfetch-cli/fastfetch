#pragma once

#include "fastfetch.h"
#include "modules/cpuusage/option.h"

typedef struct FFCpuUsageInfo {
    uint64_t inUseAll;
    uint64_t totalAll;
} FFCpuUsageInfo;
const char* ffGetCpuUsageInfo(FFlist* cpuTimes);

const char* ffGetCpuUsageResult(FFCPUUsageOptions* options, FFlist* result); // list of double
