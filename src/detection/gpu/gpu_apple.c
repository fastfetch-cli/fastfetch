#include "gpu.h"
#include "common/library.h"
#include "detection/cpu/cpu.h"
#include "detection/temps/temps_apple.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/graphics/IOGraphicsLib.h>

static double detectGpuTemp(const FFstrbuf* gpuName)
{
    FF_LIST_AUTO_DESTROY temps;
    ffListInit(&temps, sizeof(FFTempValue));

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

const char* ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(instance);

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

        ffStrbufInitA(&gpu->vendor, 0);

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

        if(instance->config.gpuTemp)
            gpu->temperature = detectGpuTemp(&gpu->name);
        else
            gpu->temperature = FF_GPU_TEMP_UNSET;

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);
    return NULL;
}
