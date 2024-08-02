#pragma once

#include "fastfetch.h"

#define FF_GPU_TEMP_UNSET (0/0.0)
#define FF_GPU_CORE_COUNT_UNSET -1
#define FF_GPU_VMEM_SIZE_UNSET ((uint64_t)-1)
#define FF_GPU_FREQUENCY_UNSET 0
#define FF_GPU_CORE_USAGE_UNSET (0/0.0)

extern const char* FF_GPU_VENDOR_NAME_APPLE;
extern const char* FF_GPU_VENDOR_NAME_AMD;
extern const char* FF_GPU_VENDOR_NAME_INTEL;
extern const char* FF_GPU_VENDOR_NAME_NVIDIA;
extern const char* FF_GPU_VENDOR_NAME_MTHREADS;
extern const char* FF_GPU_VENDOR_NAME_VMWARE;
extern const char* FF_GPU_VENDOR_NAME_PARALLEL;
extern const char* FF_GPU_VENDOR_NAME_MICROSOFT;
extern const char* FF_GPU_VENDOR_NAME_REDHAT;
extern const char* FF_GPU_VENDOR_NAME_ORACLE;

typedef struct FFGPUMemory
{
    uint64_t total;
    uint64_t used;
} FFGPUMemory;

typedef struct FFGPUResult
{
    FFGPUType type;
    FFstrbuf vendor;
    FFstrbuf name;
    FFstrbuf driver;
    FFstrbuf platformApi;
    double temperature;
    int32_t coreCount;
    uint32_t frequency; // Maximum time clock frequency in MHz
    FFGPUMemory dedicated;
    FFGPUMemory shared;
    double coreUsage;
    uint64_t deviceId; // Used internally, may be uninitialized
} FFGPUResult;

const char* ffDetectGPU(const FFGPUOptions* options, FFlist* result);
const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus);

const char* ffGetGPUVendorString(unsigned vendorId);

#if defined(__linux__) || defined(__FreeBSD__) || defined(__sun)
void ffGPUParsePciIds(FFstrbuf* content, uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu);
#endif
