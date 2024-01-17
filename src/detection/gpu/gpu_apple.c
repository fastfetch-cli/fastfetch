#include "gpu.h"
#include "common/library.h"
#include "detection/cpu/cpu.h"
#include "detection/temps/temps_apple.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/graphics/IOGraphicsLib.h>

const char* ffGpuDetectMetal(FFlist* gpus);

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

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    FF_IOOBJECT_AUTO_RELEASE io_iterator_t iterator = IO_OBJECT_NULL;
    if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching(kIOAcceleratorClassName), &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

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

        int vram; // Supported on Intel
        if(!ffCfDictGetInt(properties, CFSTR("VRAM,totalMB"), &vram))
            gpu->dedicated.total = (uint64_t) vram * 1024 * 1024;

        if(ffCfDictGetInt(properties, CFSTR("gpu-core-count"), &gpu->coreCount)) // For Apple
            gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;

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
        if(!ffCfDictGetInt(properties, CFSTR("vendor-id"), &vendorId))
        {
            const char* vendorStr = ffGetGPUVendorString((unsigned) vendorId);
            ffStrbufAppendS(&gpu->vendor, vendorStr);
            if (vendorStr == FF_GPU_VENDOR_NAME_APPLE || vendorStr == FF_GPU_VENDOR_NAME_INTEL)
                gpu->type = FF_GPU_TYPE_INTEGRATED;
            else if (vendorStr == FF_GPU_VENDOR_NAME_NVIDIA || vendorStr == FF_GPU_VENDOR_NAME_AMD)
                gpu->type = FF_GPU_TYPE_DISCRETE;
        }

        gpu->temperature = options->temp ? detectGpuTemp(&gpu->name) : FF_GPU_TEMP_UNSET;

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    ffGpuDetectMetal(gpus);
    return NULL;
}
