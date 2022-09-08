#include "gpu.h"
#include "common/library.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/graphics/IOGraphicsLib.h>

void ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(instance);

    void* iokit = dlopen(FASTFETCH_TARGET_DIR_ROOT"/System/Library/Frameworks/IOKit.framework/IOKit", RTLD_LAZY);
    if(iokit == NULL)
        return;

    FF_LIBRARY_LOAD_SYMBOL(iokit, IOServiceMatching, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, IOServiceGetMatchingServices, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, IOIteratorNext, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, IORegistryEntryCreateCFProperties, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, kCFAllocatorDefault, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, kCFAllocatorNull, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, CFDictionaryGetValue, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, CFGetTypeID, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, CFStringCreateWithCStringNoCopy, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, CFStringGetLength, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, CFStringGetCString, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, CFRelease, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, CFDataGetTypeID, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, IOObjectRelease, )

    CFMutableDictionaryRef matchDict = ffIOServiceMatching("IOPCIDevice");
    io_iterator_t iterator;
    if(ffIOServiceGetMatchingServices(0, matchDict, &iterator) != kIOReturnSuccess)
    {
        dlclose(iokit);
        return;
    }

    io_registry_entry_t registryEntry;
    while((registryEntry = ffIOIteratorNext(iterator)) != 0)
    {
        CFMutableDictionaryRef properties;
        if(ffIORegistryEntryCreateCFProperties(registryEntry, &properties, *ffkCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            ffIOObjectRelease(registryEntry);
            continue;
        }

        CFStringRef key = ffCFStringCreateWithCStringNoCopy(NULL, "model", kCFStringEncodingUTF8, *ffkCFAllocatorNull);
        CFStringRef model = ffCFDictionaryGetValue(properties, key);
        if(model == NULL || ffCFGetTypeID(model) != ffCFDataGetTypeID())
        {
            ffCFRelease(properties);
            ffIOObjectRelease(registryEntry);
            continue;
        }

        FFGPUResult* gpu = ffListAdd(gpus);

        uint32_t modelLength = (uint32_t) ffCFStringGetLength(model);
        ffStrbufInitA(&gpu->name, modelLength + 1);
        ffCFStringGetCString(model, gpu->name.chars, modelLength + 1, kCFStringEncodingUTF8);
        gpu->name.length = modelLength;
        gpu->name.chars[gpu->name.length] = '\0';

        ffStrbufInitA(&gpu->vendor, 0);
        ffStrbufInitA(&gpu->driver, 0);
        gpu->temperature = FF_GPU_TEMP_UNSET;

        ffCFRelease(properties);
        ffIOObjectRelease(registryEntry);
    }

    ffIOObjectRelease(iterator);
    dlclose(iokit);
}
