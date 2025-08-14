#pragma once

#include "fastfetch.h"
#include "modules/gpu/option.h"

#define FF_GPU_TEMP_UNSET (-DBL_MAX)
#define FF_GPU_CORE_COUNT_UNSET -1
#define FF_GPU_VMEM_SIZE_UNSET ((uint64_t)-1)
#define FF_GPU_FREQUENCY_UNSET 0
#define FF_GPU_CORE_USAGE_UNSET (-DBL_MAX)
#define FF_GPU_INDEX_UNSET ((uint32_t)-1)

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
    uint32_t index;
    FFGPUType type;
    FFstrbuf vendor;
    FFstrbuf name;
    FFstrbuf driver;
    FFstrbuf platformApi;
    FFstrbuf memoryType;
    double temperature;
    double coreUsage;
    int32_t coreCount;
    uint32_t frequency; // Maximum time clock frequency in MHz
    FFGPUMemory dedicated;
    FFGPUMemory shared;
    uint64_t deviceId;
} FFGPUResult;

const char* ffDetectGPU(const FFGPUOptions* options, FFlist* result);
const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus);

const char* ffGPUGetVendorString(unsigned vendorId);

typedef struct FFGpuDriverPciBusId
{
    uint32_t domain;
    uint32_t bus;
    uint32_t device;
    uint32_t func;
} FFGpuDriverPciBusId;

#if defined(__linux__) || defined(__FreeBSD__) || defined(__sun) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__HAIKU__)
void ffGPUFillVendorAndName(uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu);
void ffGPUQueryAmdGpuName(uint16_t deviceId, uint8_t revisionId, FFGPUResult* gpu);

#if FF_HAVE_DRM
const char* ffDrmDetectRadeon(const FFGPUOptions* options, FFGPUResult* gpu, const char* renderPath);
const char* ffDrmDetectAmdgpu(const FFGPUOptions* options, FFGPUResult* gpu, const char* renderPath);
const char* ffDrmDetectI915(FFGPUResult* gpu, int fd);
const char* ffDrmDetectXe(FFGPUResult* gpu, int fd);
const char* ffDrmDetectAsahi(FFGPUResult* gpu, int fd);
const char* ffDrmDetectNouveau(FFGPUResult* gpu, int fd);
#endif // FF_HAVE_DRM

const char* ffGPUDetectDriverSpecific(const FFGPUOptions* options, FFGPUResult* gpu, FFGpuDriverPciBusId pciBusId);
#endif // defined(XXX)
