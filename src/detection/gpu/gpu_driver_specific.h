#pragma once

#include "gpu.h"

typedef enum FFGpuDriverConditionType
{
    FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID = 1 << 0,
    FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID = 1 << 1,
    FF_GPU_DRIVER_CONDITION_TYPE_LUID = 1 << 2,
} FFGpuDriverConditionType;

typedef struct FFGpuDriverPciBusId
{
    uint32_t domain;
    uint32_t bus;
    uint32_t device;
    uint32_t func;
} FFGpuDriverPciBusId;

typedef struct FFGpuDriverPciDeviceId
{
    uint32_t deviceId;
    uint32_t vendorId;
    uint32_t subSystemId;
    uint32_t revId;
} FFGpuDriverPciDeviceId;

// Use pciBusId if not NULL; use pciDeviceId otherwise
typedef struct FFGpuDriverCondition
{
    FFGpuDriverConditionType type;
    FFGpuDriverPciBusId pciBusId;
    FFGpuDriverPciDeviceId pciDeviceId;
    uint64_t luid;
} FFGpuDriverCondition;

// detect x if not NULL
typedef struct FFGpuDriverResult
{
    double* temp;
    FFGPUMemory* memory;
    uint32_t* coreCount;
    double* coreUsage;
    FFGPUType* type;
    uint32_t* frequency;
    FFstrbuf* name;
} FFGpuDriverResult;

const char* ffDetectNvidiaGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName);
const char* ffDetectIntelGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName);
const char* ffDetectAmdGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName);
const char* ffDetectMthreadsGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName);

FF_MAYBE_UNUSED static inline bool getDriverSpecificDetectionFn(const char* vendor, __typeof__(&ffDetectNvidiaGpuInfo)* pDetectFn, const char** pDllName)
{
    if (vendor == FF_GPU_VENDOR_NAME_NVIDIA)
    {
        *pDetectFn = ffDetectNvidiaGpuInfo;
        #ifdef _WIN32
        *pDllName = "nvml.dll";
        #else
        *pDllName = "libnvidia-ml.so";
        #endif
    }
    else if (vendor == FF_GPU_VENDOR_NAME_MTHREADS)
    {
        *pDetectFn = ffDetectMthreadsGpuInfo;
        #ifdef _WIN32
        *pDllName = "mtml.dll";
        #else
        *pDllName = "libmtml.so";
        #endif
    }
    #ifdef _WIN32
    else if (vendor == FF_GPU_VENDOR_NAME_INTEL)
    {
        *pDetectFn = ffDetectIntelGpuInfo;
        #ifdef _WIN64
            *pDllName = "ControlLib.dll";
        #else
            *pDllName = "ControlLib32.dll";
        #endif
    }
    else if (vendor == FF_GPU_VENDOR_NAME_AMD)
    {
        *pDetectFn = ffDetectAmdGpuInfo;
        #ifdef _WIN64
            *pDllName = "amd_ags_x64.dll";
        #else
            *pDllName = "amd_ags_x86.dll";
        #endif
    }
    #endif
    else
    {
        *pDetectFn = NULL;
        *pDllName = NULL;
        return false;
    }

    return true;
}
