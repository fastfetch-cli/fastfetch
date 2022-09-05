#include "gpu.h"
#include "common/library.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/graphics/IOGraphicsLib.h>

void ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(instance);

    void* iokit = dlopen("/System/Library/Frameworks/IOKit.framework/IOKit", RTLD_LAZY);
    if(iokit == NULL)
        return;

    FF_LIBRARY_LOAD_SYMBOL(iokit, IOServiceMatching, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, IOServiceGetMatchingServices, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, kIOMasterPortDefault, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, IOIteratorNext, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, IORegistryEntryGetName, )
    FF_LIBRARY_LOAD_SYMBOL(iokit, IOObjectRelease, )

    CFMutableDictionaryRef matchDict = ffIOServiceMatching(kIOAcceleratorClassName);
    io_iterator_t iterator;
    if(ffIOServiceGetMatchingServices(*ffkIOMasterPortDefault, matchDict, &iterator) != kIOReturnSuccess)
    {
        dlclose(iokit);
        return;
    }

    io_registry_entry_t registryEntry;
    while((registryEntry = ffIOIteratorNext(iterator)) != 0)
    {
        io_name_t deviceName;
        kern_return_t ret = ffIORegistryEntryGetName(registryEntry, deviceName);
        ffIOObjectRelease(registryEntry);

        if(ret != KERN_SUCCESS)
            continue;

        FFGPUResult* gpu = ffListAdd(gpus);

        ffStrbufInit(&gpu->name);
        ffStrbufAppendS(&gpu->name, deviceName);

        ffStrbufInitA(&gpu->vendor, 0);
        ffStrbufInitA(&gpu->driver, 0);
        gpu->temperature = FF_GPU_TEMP_UNSET;
    }

    ffIOObjectRelease(iterator);
    dlclose(iokit);
}
