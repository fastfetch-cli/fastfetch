#pragma once

#ifndef FF_INCLUDED_detection_cpu_cpuUsage
#define FF_INCLUDED_detection_cpu_cpuUsage

#if defined(_WIN32) || defined(__MSYS__)
    // Disabled by default because the result does need some time to generate
    #define FF_DETECTION_CPUUSAGE_NOWAIT 0
#endif

const char* ffGetCpuUsageResult(double* result);

#endif
