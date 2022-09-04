#pragma once

#ifndef FF_INCLUDED_detection_cpu_cpu
#define FF_INCLUDED_detection_cpu_cpu

#include "fastfetch.h"

#define FF_CPU_TEMP_UNSET (0/0.0)

typedef struct FFCPUResult
{
    FFstrbuf name;
    FFstrbuf vendor;

    uint16_t coresPhysical;
    uint16_t coresLogical;
    uint16_t coresOnline;

    double frequencyMin;
    double frequencyMax;

    double temperature;
} FFCPUResult;

const FFCPUResult* ffDetectCPU();

#endif
