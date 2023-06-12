#include "fastfetch.h"
#include "battery.h"
#include "util/apple/cf_helpers.h"
#include "detection/temps/temps_apple.h"

#include <IOKit/IOKitLib.h>

static double detectBatteryTemp()
{
    FF_LIST_AUTO_DESTROY temps = ffListCreate(sizeof(FFTempValue));

    ffDetectCoreTemps(FF_TEMP_BATTERY, &temps);

    if(temps.length == 0)
        return FF_BATTERY_TEMP_UNSET;

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

const char* ffDetectBattery(FF_MAYBE_UNUSED FFinstance* instance, FFBatteryOptions* options, FFlist* results)
{
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
        battery->capacity = 0.0/0.0;
        int currentCapacity, maxCapacity;

        if ((error = ffCfDictGetInt(properties, CFSTR("MaxCapacity"), &maxCapacity)))
            return error;
        if (maxCapacity <= 0)
            return "Querying MaxCapacity failed";

        if ((error = ffCfDictGetInt(properties, CFSTR("CurrentCapacity"), &currentCapacity)))
            return error;
        if(currentCapacity <= 0)
            return "Querying CurrentCapacity failed";

        battery->capacity = currentCapacity * 100.0 / maxCapacity;

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

        if(options->temp)
            battery->temperature = detectBatteryTemp();
        else
            battery->temperature = FF_BATTERY_TEMP_UNSET;

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);

    return NULL;
}
