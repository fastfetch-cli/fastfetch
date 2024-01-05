#include "fastfetch.h"
#include "battery.h"
#include "util/apple/cf_helpers.h"
#include "detection/temps/temps_apple.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/pwr_mgt/IOPM.h>

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching("AppleSmartBattery"), &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

    io_registry_entry_t registryEntry;
    while((registryEntry = IOIteratorNext(iterator)) != 0)
    {
        FF_CFTYPE_AUTO_RELEASE CFMutableDictionaryRef properties = NULL;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            continue;
        }

        bool boolValue;
        const char* error;

        FFBatteryResult* battery = ffListAdd(results);
        battery->temperature = FF_BATTERY_TEMP_UNSET;
        ffStrbufInit(&battery->manufacturer);
        ffStrbufInit(&battery->modelName);
        ffStrbufInit(&battery->serial);
        ffStrbufInit(&battery->technology);
        ffStrbufInit(&battery->status);
        battery->capacity = 0.0/0.0;

        int currentCapacity, maxCapacity;

        if ((error = ffCfDictGetInt(properties, CFSTR(kIOPMPSMaxCapacityKey), &maxCapacity)))
            return error;
        if (maxCapacity <= 0)
            return "Querying MaxCapacity failed";

        if ((error = ffCfDictGetInt(properties, CFSTR(kIOPMPSCurrentCapacityKey), &currentCapacity)))
            return error;
        if(currentCapacity <= 0)
            return "Querying CurrentCapacity failed";

        battery->capacity = currentCapacity * 100.0 / maxCapacity;

        ffCfDictGetString(properties, CFSTR(kIOPMDeviceNameKey), &battery->modelName);
        ffCfDictGetString(properties, CFSTR(kIOPMPSSerialKey), &battery->serial);

        if (!ffCfDictGetBool(properties, CFSTR("built-in"), &boolValue) && boolValue)
        {
            ffStrbufAppendS(&battery->manufacturer, "Apple Inc.");
            ffStrbufAppendS(&battery->technology, "Lithium");
            if (!battery->modelName.length)
                ffStrbufAppendS(&battery->modelName, "Built-in");
        }

        int32_t cycleCount = 0;
        ffCfDictGetInt(properties, CFSTR(kIOPMPSCycleCountKey), &cycleCount);
        battery->cycleCount = cycleCount < 0 ? 0 : (uint32_t) cycleCount;

        if (!ffCfDictGetBool(properties, CFSTR(kIOPMPSExternalConnectedKey), &boolValue) && boolValue)
            ffStrbufAppendS(&battery->status, "AC connected, ");
        else
            ffStrbufAppendS(&battery->status, "Discharging, ");
        if (!ffCfDictGetBool(properties, CFSTR(kIOPMPSIsChargingKey), &boolValue) && boolValue)
            ffStrbufAppendS(&battery->status, "Charging, ");
        if (!ffCfDictGetBool(properties, CFSTR(kIOPMPSAtCriticalLevelKey), &boolValue) && boolValue)
            ffStrbufAppendS(&battery->status, "Critical, ");
        ffStrbufTrimRight(&battery->status, ' ');
        ffStrbufTrimRight(&battery->status, ',');

        if (options->temp)
        {
            int64_t temp;
            if (!ffCfDictGetInt64(properties, CFSTR(kIOPMPSBatteryTemperatureKey), &temp))
                battery->temperature = (double) temp / 10 - 273.15;
            else
                ffDetectSmcTemps(FF_TEMP_BATTERY, &battery->temperature);
        }

        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);

    return NULL;
}
