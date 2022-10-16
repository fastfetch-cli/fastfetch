extern "C" {
#include "battery.h"
}
#include "util/windows/wmi.hpp"

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results)
{
    FF_UNUSED(instance);

    //https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/win32-battery
    FFWmiQuery query(L"SELECT SystemName, Name, Chemistry, EstimatedChargeRemaining, BatteryStatus FROM Win32_Battery");

    if(!query)
        return "Query WMI service failed";

    while(FFWmiRecord record = query.next())
    {
        BatteryResult* battery = (BatteryResult*)ffListAdd(results);

        ffStrbufInit(&battery->manufacturer);
        record.getString(L"SystemName", &battery->manufacturer);

        ffStrbufInit(&battery->modelName);
        record.getString(L"Name", &battery->modelName);

        uint64_t chemistry = 0;
        record.getUnsigned(L"Chemistry", &chemistry);
        switch(chemistry)
        {
            case 1: ffStrbufInitS(&battery->technology, "Other"); break;
            case 2: ffStrbufInitS(&battery->technology, "Unknown"); break;
            case 3: ffStrbufInitS(&battery->technology, "Lead Acid"); break;
            case 4: ffStrbufInitS(&battery->technology, "Nickel Cadmium"); break;
            case 5: ffStrbufInitS(&battery->technology, "Nickel Metal Hydride"); break;
            case 6: ffStrbufInitS(&battery->technology, "Lithium-ion"); break;
            case 7: ffStrbufInitS(&battery->technology, "Zinc air"); break;
            case 8: ffStrbufInitS(&battery->technology, "Lithium Polymer"); break;
            default: ffStrbufInit(&battery->technology); break;
        }

        uint64_t capacity;
        record.getUnsigned(L"EstimatedChargeRemaining", &capacity);
        ffStrbufInitF(&battery->capacity, "%d", (int)capacity);

        uint64_t batteryStatus;
        record.getUnsigned(L"BatteryStatus", &batteryStatus);
        switch(batteryStatus)
        {
            case 1: ffStrbufInitS(&battery->status, "Discharging"); break;
            case 2: ffStrbufInitS(&battery->status, "AC Connected"); break;
            case 3: ffStrbufInitS(&battery->status, "Fully Charged"); break;
            case 4: ffStrbufInitS(&battery->status, "Low"); break;
            case 5: ffStrbufInitS(&battery->status, "Critical"); break;
            case 6: ffStrbufInitS(&battery->status, "Charging"); break;
            case 7: ffStrbufInitS(&battery->status, "Charging and High"); break;
            case 8: ffStrbufInitS(&battery->status, "Charging and Low"); break;
            case 9: ffStrbufInitS(&battery->status, "Charging and Critical"); break;
            case 10: ffStrbufInitS(&battery->status, "Undefined"); break;
            case 11: ffStrbufInitS(&battery->status, "Partially Charged"); break;
            default: ffStrbufInit(&battery->status); break;
        }

        battery->temperature = FF_BATTERY_TEMP_UNSET;
    }

    return nullptr;
}
