#include "gpu.h"
#include "common/library.h"
#include "detection/cpu/cpu.h"
#include "util/apple/cf_helpers.h"
#include "util/apple/smc_temps.h"

#include <IOKit/graphics/IOGraphicsLib.h>

const char* ffGpuDetectMetal(FFlist* gpus);
const char* ffGpuDetectDriverVersion(FFlist* gpus);

static double detectGpuTemp(const FFstrbuf* gpuName)
{
    double result = 0;
    const char* error = NULL;

    if (ffStrbufStartsWithS(gpuName, "Apple M"))
    {
        switch (strtol(gpuName->chars + strlen("Apple M"), NULL, 10))
        {
            case 1: error = ffDetectSmcTemps(FF_TEMP_GPU_M1X, &result); break;
            case 2: error = ffDetectSmcTemps(FF_TEMP_GPU_M2X, &result); break;
            case 3: error = ffDetectSmcTemps(FF_TEMP_GPU_M3X, &result); break;
            default: error = "Unsupported Apple Silicon GPU";
        }
    }
    else if (ffStrbufStartsWithS(gpuName, "Intel"))
        error = ffDetectSmcTemps(FF_TEMP_GPU_INTEL, &result);
    else if (ffStrbufStartsWithS(gpuName, "Radeon") || ffStrbufStartsWithS(gpuName, "AMD"))
        error = ffDetectSmcTemps(FF_TEMP_GPU_AMD, &result);
    else
        error = ffDetectSmcTemps(FF_TEMP_GPU_UNKNOWN, &result);

    if (error)
        return FF_GPU_TEMP_UNSET;

    return result;
}

#ifdef __aarch64__
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

static const char* detectFrequency(FFGPUResult* gpu)
{
    // https://github.com/giampaolo/psutil/pull/2222/files

    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t entryDevice = IOServiceGetMatchingService(MACH_PORT_NULL, IOServiceNameMatching("pmgr"));
    if (!entryDevice)
        return "IOServiceGetMatchingServices() failed";

    if (!IOObjectConformsTo(entryDevice, "AppleARMIODevice"))
        return "\"pmgr\" should conform to \"AppleARMIODevice\"";

    FF_CFTYPE_AUTO_RELEASE CFDataRef freqProperty = (CFDataRef) IORegistryEntryCreateCFProperty(entryDevice, CFSTR("voltage-states9-sram"), kCFAllocatorDefault, kNilOptions);
    if (!freqProperty || CFGetTypeID(freqProperty) != CFDataGetTypeID())
        return "\"voltage-states9-sram\" in \"pmgr\" is not found";

    // voltage-states5-sram stores supported <frequency / voltage> pairs of gpu from the lowest to the highest
    CFIndex propLength = CFDataGetLength(freqProperty);
    if (propLength == 0 || propLength % (CFIndex) sizeof(uint32_t) * 2 != 0)
        return "Invalid \"voltage-states9-sram\" length";

    uint32_t* pStart = (uint32_t*) CFDataGetBytePtr(freqProperty);
    uint32_t pMax = *pStart;
    for (CFIndex i = 2; i < propLength / (CFIndex) sizeof(uint32_t) && pStart[i] > 0; i += 2 /* skip voltage */)
        pMax = pMax > pStart[i] ? pMax : pStart[i];

    if (pMax > 0)
        gpu->frequency = pMax / 1000 / 1000;

    return NULL;
}
#endif

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    FF_IOOBJECT_AUTO_RELEASE io_iterator_t iterator = IO_OBJECT_NULL;
    {
        CFMutableDictionaryRef matches = IOServiceMatching(kIOAcceleratorClassName);
        CFDictionaryAddValue(matches, CFSTR("IOMatchCategory"), CFSTR(kIOAcceleratorClassName));
        if (IOServiceGetMatchingServices(MACH_PORT_NULL, matches, &iterator) != kIOReturnSuccess)
            return "IOServiceGetMatchingServices() failed";
    }

    io_registry_entry_t registryEntry;
    while ((registryEntry = IOIteratorNext(iterator)) != IO_OBJECT_NULL)
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            continue;
        }

        FFGPUResult* gpu = ffListAdd(gpus);

        gpu->dedicated.total = gpu->dedicated.used = gpu->shared.total = gpu->shared.used = FF_GPU_VMEM_SIZE_UNSET;
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;
        IORegistryEntryGetRegistryEntryID(registryEntry, &gpu->deviceId);
        ffStrbufInitStatic(&gpu->platformApi, "Metal");

        ffStrbufInit(&gpu->driver); // Ok for both Apple and Intel
        ffCfDictGetString(properties, CFSTR("CFBundleIdentifier"), &gpu->driver);

        if(ffCfDictGetInt(properties, CFSTR("gpu-core-count"), &gpu->coreCount)) // For Apple
            gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;

        gpu->coreUsage = 0.0/0.0;
        CFDictionaryRef perfStatistics = NULL;
        uint64_t vramUsed = 0, vramTotal = 0;
        if (ffCfDictGetDict(properties, CFSTR("PerformanceStatistics"), &perfStatistics) == NULL)
        {
            int64_t utilization;
            if (ffCfDictGetInt64(perfStatistics, CFSTR("Device Utilization %"), &utilization) == NULL)
                gpu->coreUsage = (double) utilization;
            else if (ffCfDictGetInt64(perfStatistics, CFSTR("GPU Core Utilization"), &utilization) == NULL)
                gpu->coreUsage = (double) utilization / 10000000.; // Nvidia?

            if (ffCfDictGetInt64(perfStatistics, CFSTR("Alloc system memory"), (int64_t*) &vramTotal) == NULL)
            {
                if (ffCfDictGetInt64(perfStatistics, CFSTR("In use system memory"), (int64_t*) &vramUsed) != NULL)
                    vramTotal = 0;
            }
            else if (ffCfDictGetInt64(perfStatistics, CFSTR("vramUsedBytes"), (int64_t*) &vramTotal) == NULL)
            {
                if (ffCfDictGetInt64(perfStatistics, CFSTR("vramFreeBytes"), (int64_t*) &vramUsed) == NULL)
                    vramTotal += vramUsed;
                else
                    vramTotal = 0;
            }
        }

        ffStrbufInit(&gpu->name);
        //IOAccelerator returns model / vendor-id properties for Apple Silicon, but not for Intel Iris GPUs.
        //Still needs testing for AMD's
        if(ffCfDictGetString(properties, CFSTR("model"), &gpu->name))
        {
            CFRelease(properties);

            io_registry_entry_t parentEntry;
            IORegistryEntryGetParentEntry(registryEntry, kIOServicePlane, &parentEntry);
            if(IORegistryEntryCreateCFProperties(parentEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
            {
                IOObjectRelease(parentEntry);
                IOObjectRelease(registryEntry);
                continue;
            }
            ffCfDictGetString(properties, CFSTR("model"), &gpu->name);
        }

        ffStrbufInit(&gpu->vendor);
        int vendorId;
        if(ffCfDictGetInt(properties, CFSTR("vendor-id"), &vendorId) == NULL)
        {
            const char* vendorStr = ffGetGPUVendorString((unsigned) vendorId);
            ffStrbufAppendS(&gpu->vendor, vendorStr);
            if (vendorStr == FF_GPU_VENDOR_NAME_APPLE || vendorStr == FF_GPU_VENDOR_NAME_INTEL)
                gpu->type = FF_GPU_TYPE_INTEGRATED;
            else if (vendorStr == FF_GPU_VENDOR_NAME_NVIDIA || vendorStr == FF_GPU_VENDOR_NAME_AMD)
                gpu->type = FF_GPU_TYPE_DISCRETE;

            #ifdef __aarch64__
            if (vendorStr == FF_GPU_VENDOR_NAME_APPLE)
                detectFrequency(gpu);
            #endif

            if (gpu->type == FF_GPU_TYPE_INTEGRATED)
            {
                gpu->shared.total = vramTotal;
                gpu->shared.used = vramUsed;
            }
            else
            {
                gpu->dedicated.total = vramTotal;
                gpu->dedicated.used = vramUsed;
            }
        }

        gpu->temperature = options->temp ? detectGpuTemp(&gpu->name) : FF_GPU_TEMP_UNSET;

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    ffGpuDetectMetal(gpus);
    if (instance.config.general.detectVersion)
        ffGpuDetectDriverVersion(gpus);
    return NULL;
}
