#include "fastfetch.h"
#include "battery.h"
#include "util/apple/cf_helpers.h"

#include <IOKit/IOKitLib.h>

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results)
{
    FF_UNUSED(instance);

    CFMutableDictionaryRef matchDict = IOServiceMatching("AppleSmartBattery");
    if (matchDict == NULL)
        return "IOServiceMatching(\"AppleSmartBattery\") failed";

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

        bool boolValue;
        const char* error;

        BatteryResult* battery = ffListAdd(results);
        ffStrbufInit(&battery->capacity);
        int currentCapacity, maxCapacity;

        if ((error = ffCfDictGetInt(properties, CFSTR("MaxCapacity"), &maxCapacity)))
            return error;
        if (maxCapacity <= 0)
            return "Querying MaxCapacity failed";

        if ((error = ffCfDictGetInt(properties, CFSTR("CurrentCapacity"), &currentCapacity)))
            return error;
        if(currentCapacity <= 0)
            return "Querying CurrentCapacity failed";

        ffStrbufAppendF(&battery->capacity, "%.0f", currentCapacity * 100.0 / maxCapacity);

        ffStrbufInit(&battery->manufacturer);
        ffStrbufInit(&battery->modelName);
        ffStrbufInit(&battery->technology);
        if (!ffCfDictGetBool(properties, CFSTR("built-in"), &boolValue) && boolValue)
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
        if (!ffCfDictGetBool(properties, CFSTR("FullyCharged"), &boolValue) && boolValue)
            ffStrbufAppendS(&battery->status, "Fully charged");
        else if (!ffCfDictGetBool(properties, CFSTR("IsCharging"), &boolValue) && boolValue)
            ffStrbufAppendS(&battery->status, "Charging");
        else
            ffStrbufAppendS(&battery->status, "");

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);

    return NULL;
}
