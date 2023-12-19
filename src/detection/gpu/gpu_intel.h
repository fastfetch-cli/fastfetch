#pragma once

#include "gpu.h"

// Use pciBusId if not NULL; use pciDeviceId and pciSubSystemId otherwise
typedef struct FFGpuIntelCondition
{
    uint32_t pciDeviceId;
    uint32_t pciVendorId;
    uint32_t pciSubSystemId;
    uint32_t revId;
} FFGpuIntelCondition;

// detect x if not NULL
typedef struct FFGpuIntelResult
{
    double* temp;
    FFGPUMemory* memory;
    uint32_t* coreCount;
    FFGPUType* type;
} FFGpuIntelResult;

const char* ffDetectIntelGpuInfo(FFGpuIntelCondition cond, FFGpuIntelResult result, const char* soName);
