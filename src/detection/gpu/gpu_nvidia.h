#pragma once

#include "gpu.h"

// Use pciBusId if not NULL; use pciDeviceId and pciSubSystemId otherwise
// pciBusId = "domain:bus:device.function"
// pciDeviceId = (deviceId << 16) | vendorId
typedef struct FFGpuNvidiaCondition
{
    const char* pciBusId;
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

const char* ffDetectNvidiaGpuInfo(FFGpuNvidiaCondition cond, FFGpuNvidiaResult result);
