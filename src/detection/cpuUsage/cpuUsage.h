#pragma once

#ifndef FF_INCLUDED_detection_cpu_cpuUsage
#define FF_INCLUDED_detection_cpu_cpuUsage

#if defined(_WIN32) || defined(__MSYS__)
    #define FF_DETECTION_CPUUSAGE_NOWAIT 1
#endif

const char* ffGetCpuUsageResult(double* result);

#endif
