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
    FFGPUType* type;
    uint32_t* frequency;
} FFGpuDriverResult;

const char* ffDetectNvidiaGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName);
const char* ffDetectIntelGpuInfo(const FFGpuDriverCondition* cond, FFGpuDriverResult result, const char* soName);
