#pragma once

#include "gpu.h"

typedef enum FF_A_PACKED FFGpuDriverConditionType {
    FF_GPU_DRIVER_CONDITION_TYPE_BUS_ID = 1 << 0,
    FF_GPU_DRIVER_CONDITION_TYPE_DEVICE_ID = 1 << 1,
    FF_GPU_DRIVER_CONDITION_TYPE_LUID = 1 << 2,
    FF_GPU_DRIVER_CONDITION_TYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFGpuDriverConditionType;

typedef struct FFGpuDriverPciDeviceId {
    uint32_t deviceId;
    uint32_t vendorId;
    uint32_t subSystemId;
    uint32_t revId;
} FFGpuDriverPciDeviceId;

// Use pciBusId if not NULL; use pciDeviceId otherwise
typedef struct FFGpuDriverCondition {
    FFGpuDriverConditionType type;
    FFGpuDriverPciBusId pciBusId;
    FFGpuDriverPciDeviceId pciDeviceId;
    uint64_t luid;
} FFGpuDriverCondition;

// detect x if not NULL
typedef struct FFGpuDriverResult {
    uint32_t* index;
    double* temp;
    FFGPUMemory* memory;
    FFstrbuf* memoryType;
    FFGPUMemory* sharedMemory;
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

#ifndef FF_GPU_DRIVER_DLLNAME_PATH_PREFIX
#    define FF_GPU_DRIVER_DLLNAME_PATH_PREFIX
#endif

FF_A_UNUSED static inline bool getDriverSpecificDetectionFn(const char* vendor, __typeof__(&ffDetectNvidiaGpuInfo)* pDetectFn, const char** pDllName) {
    if (vendor == FF_GPU_VENDOR_NAME_NVIDIA) {
        *pDetectFn = ffDetectNvidiaGpuInfo;
#ifdef _WIN32
        *pDllName = FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "nvml.dll";
#else
        *pDllName = FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "libnvidia-ml.so";
#endif
    } else if (vendor == FF_GPU_VENDOR_NAME_MTHREADS) {
        *pDetectFn = ffDetectMthreadsGpuInfo;
#ifdef _WIN32
        *pDllName = FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "mtml.dll";
#else
        *pDllName = FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "libmtml.so";
#endif
    }
#ifdef _WIN32
    else if (vendor == FF_GPU_VENDOR_NAME_INTEL) {
        *pDetectFn = ffDetectIntelGpuInfo;
#    ifdef _WIN64
        *pDllName = FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "ControlLib.dll";
#    else
        *pDllName = FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "ControlLib32.dll";
#    endif
    } else if (vendor == FF_GPU_VENDOR_NAME_AMD) {
        *pDetectFn = ffDetectAmdGpuInfo;
#    ifdef _WIN64
        *pDllName = FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "atiadlxx.dll";
#    else
        *pDllName = FF_GPU_DRIVER_DLLNAME_PATH_PREFIX "atiadlxy.dll";
#    endif
    }
#endif
    else {
        *pDetectFn = NULL;
        *pDllName = NULL;
        return false;
    }

    return true;
}
