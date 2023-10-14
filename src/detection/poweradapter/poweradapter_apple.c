#include "fastfetch.h"
#include "poweradapter.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

const char* ffDetectPowerAdapterImpl(FFlist* results)
{
    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching("AppleSmartBattery"), &iterator) != kIOReturnSuccess)
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

        FFPowerAdapterResult* adapter = ffListAdd(results);

        ffStrbufInit(&adapter->name);
        ffStrbufInit(&adapter->description);
        ffStrbufInit(&adapter->manufacturer);
        ffStrbufInit(&adapter->modelName);
        adapter->watts = FF_POWERADAPTER_UNSET;

        CFDictionaryRef adapterDict;
        if(!ffCfDictGetDict(properties, CFSTR("AdapterDetails"), &adapterDict))
        {
            if (ffCfDictGetInt(adapterDict, CFSTR("Watts"), &adapter->watts))
            {
                adapter->watts = FF_POWERADAPTER_NOT_CONNECTED;
                continue;
            }
            ffCfDictGetString(adapterDict, CFSTR("Name"), &adapter->name);
            ffCfDictGetString(adapterDict, CFSTR("Description"), &adapter->description);
            ffCfDictGetString(adapterDict, CFSTR("Manufacturer"), &adapter->manufacturer);
            ffCfDictGetString(adapterDict, CFSTR("Model"), &adapter->modelName);
        }

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);

    return NULL;
}
