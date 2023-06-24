#include "gpu.h"
#include "common/library.h"
#include "detection/cpu/cpu.h"
#include "detection/temps/temps_apple.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/graphics/IOGraphicsLib.h>
#include <wchar.h>

static double detectGpuTemp(const FFstrbuf* gpuName)
{
    FF_LIST_AUTO_DESTROY temps = ffListCreate(sizeof(FFTempValue));

    if(ffStrbufStartsWithS(gpuName, "Apple M1"))
        ffDetectCoreTemps(FF_TEMP_GPU_M1X, &temps);
    else if(ffStrbufStartsWithS(gpuName, "Apple M2"))
        ffDetectCoreTemps(FF_TEMP_GPU_M2X, &temps);
    else if(ffStrbufStartsWithS(gpuName, "Intel"))
        ffDetectCoreTemps(FF_TEMP_GPU_INTEL, &temps);
    else if(ffStrbufStartsWithS(gpuName, "Radeon") || ffStrbufStartsWithS(gpuName, "AMD"))
        ffDetectCoreTemps(FF_TEMP_GPU_AMD, &temps);
    else
        ffDetectCoreTemps(FF_TEMP_GPU_UNKNOWN, &temps);

    if(temps.length == 0)
        return FF_GPU_TEMP_UNSET;

    double result = 0;
    for(uint32_t i = 0; i < temps.length; ++i)
    {
        FFTempValue* tempValue = (FFTempValue*)ffListGet(&temps, i);
        result += tempValue->value;
        //TODO: do we really need this?
        ffStrbufDestroy(&tempValue->name);
        ffStrbufDestroy(&tempValue->deviceClass);
    }
    result /= temps.length;
    return result;
}

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    CFMutableDictionaryRef matchDict = IOServiceMatching(kIOAcceleratorClassName);
    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(MACH_PORT_NULL, matchDict, &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

    io_registry_entry_t registryEntry;
    while((registryEntry = IOIteratorNext(iterator)) != 0)
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

        ffStrbufInit(&gpu->driver);
        ffCfDictGetString(properties, CFSTR("CFBundleIdentifier"), &gpu->driver);

        ffStrbufInit(&gpu->name);
        //IOAccelerator returns model property for Apple Silicon, but not for Intel Iris GPUs.
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

        if(ffCfDictGetInt(properties, CFSTR("gpu-core-count"), &gpu->coreCount))
            gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;

        if(options->temp)
            gpu->temperature = detectGpuTemp(&gpu->name);
        else
            gpu->temperature = FF_GPU_TEMP_UNSET;

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);
    return NULL;
}
