#include "gpu.h"
#include "common/library.h"
#include "detection/cpu/cpu.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/graphics/IOGraphicsLib.h>

void ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(instance);

    const FFCPUResult* cpu = ffDetectCPU();
    if(ffStrbufStartsWithIgnCaseS(&cpu->name, "Apple M"))
    {
        FFGPUResult* gpu = ffListAdd(gpus);

        ffStrbufInit(&gpu->vendor);
        ffStrbufAppendS(&gpu->vendor, "Apple");

        ffStrbufInit(&gpu->name);
        ffStrbufAppendS(&gpu->name, cpu->name.chars + 6); //Cut "Apple "

        ffStrbufInitA(&gpu->driver, 0);
        gpu->temperature = FF_GPU_TEMP_UNSET;
    }

    CFMutableDictionaryRef matchDict = IOServiceMatching("IOPCIDevice");
    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(0, matchDict, &iterator) != kIOReturnSuccess)
        return;

    io_registry_entry_t registryEntry;
    while((registryEntry = IOIteratorNext(iterator)) != 0)
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            continue;
        }

        CFStringRef key = CFStringCreateWithCStringNoCopy(NULL, "model", kCFStringEncodingASCII, kCFAllocatorNull);
        CFStringRef model = CFDictionaryGetValue(properties, key);
        if(model == NULL || CFGetTypeID(model) != CFDataGetTypeID())
        {
            CFRelease(properties);
            IOObjectRelease(registryEntry);
            continue;
        }

        FFGPUResult* gpu = ffListAdd(gpus);

        uint32_t modelLength = (uint32_t) CFStringGetLength(model);
        ffStrbufInitA(&gpu->name, modelLength + 1);
        CFStringGetCString(model, gpu->name.chars, modelLength + 1, kCFStringEncodingASCII);
        gpu->name.length = modelLength;
        gpu->name.chars[gpu->name.length] = '\0';

        ffStrbufInitA(&gpu->vendor, 0);
        ffStrbufInitA(&gpu->driver, 0);
        gpu->temperature = FF_GPU_TEMP_UNSET;

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);
}
