#include "fastfetch.h"
#include "battery.h"
#include "util/apple/cfdict_helpers.h"

#include <IOKit/IOKitLib.h>

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results)
{
    FF_UNUSED(instance);

    CFMutableDictionaryRef matchDict = IOServiceMatching("AppleSmartBattery");
    if (matchDict == NULL)
        return "IOServiceMatching(\"AppleSmartBattery\") failed";

    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(0, matchDict, &iterator) != kIOReturnSuccess)
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

        int intValue;
        bool boolValue;

        BatteryResult* battery = ffListAdd(results);
        ffStrbufInit(&battery->capacity);
        if(ffCfDictGetInt(properties, "CurrentCapacity", &intValue))
            ffStrbufAppendF(&battery->capacity, "%d", intValue);

        ffStrbufInit(&battery->manufacturer);
        ffStrbufInit(&battery->modelName);
        ffStrbufInit(&battery->technology);
        if (ffCfDictGetBool(properties, "built-in", &boolValue) && boolValue)
        {
            ffStrbufAppendS(&battery->manufacturer, "Apple Inc.");
            ffStrbufAppendS(&battery->modelName, "Builtin");
            ffStrbufAppendS(&battery->technology, "Lithium");
        }
        else
        {
            ffStrbufAppendS(&battery->manufacturer, "Unknown");
            ffStrbufAppendS(&battery->modelName, "Unknown");
            ffStrbufAppendS(&battery->technology, "Unknown");
        }

        ffStrbufInit(&battery->status);
        if (ffCfDictGetBool(properties, "FullyCharged", &boolValue) && boolValue)
            ffStrbufAppendS(&battery->status, "Fully charged");
        else if (ffCfDictGetBool(properties, "IsCharging", &boolValue) && boolValue)
            ffStrbufAppendS(&battery->status, "Charging");
        else
            ffStrbufAppendS(&battery->status, "");

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);

    return NULL;
}
