#pragma once

#ifndef FF_INCLUDED_detection_cpu_cpuUsage
#define FF_INCLUDED_detection_cpu_cpuUsage

#include <stdint.h>

// We need to use uint64_t because sizeof(long) == 4 on Windows
const char* ffGetCpuUsageInfo(uint64_t* inUseAll, uint64_t* totalAll);

#endif
