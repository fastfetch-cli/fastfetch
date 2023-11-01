#pragma once

#include "gpu.h"

// Use pciBusId if not NULL; use pciDeviceId and pciSubSystemId otherwise
typedef struct FFGpuNvidiaCondition
{
    // "domain:bus:device.function"
    const char* pciBusId;
    // (deviceId << 16) | vendorId
    uint32_t pciDeviceId;
    uint32_t pciSubSystemId;
} FFGpuNvidiaCondition;

// detect x if not NULL
typedef struct FFGpuNvidiaResult
{
    double* temp;
    FFGPUMemory* memory;
    uint32_t* coreCount;
} FFGpuNvidiaResult;

const char* ffDetectNvidiaGpuInfo(FFGpuNvidiaCondition cond, FFGpuNvidiaResult result, const char* soName);
