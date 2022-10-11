extern "C" {
#include "battery.h"
}
#include "util/windows/wmi.hpp"

const char* ffDetectBatteryImpl(FFinstance* instance, FFlist* results)
{
    FF_UNUSED(instance);

    //https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/win32-battery
    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT SystemName, Name, Chemistry, EstimatedChargeRemaining, BatteryStatus FROM Win32_Battery", nullptr);

    if(!pEnumerator)
        return "Query WMI service failed";

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while(SUCCEEDED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) && uReturn != 0)
    {
        BatteryResult* battery = (BatteryResult*)ffListAdd(results);

        ffStrbufInit(&battery->manufacturer);
        ffGetWmiObjString(pclsObj, L"SystemName", &battery->manufacturer);

        ffStrbufInit(&battery->modelName);
        ffGetWmiObjString(pclsObj, L"Name", &battery->modelName);

        uint64_t chemistry = 0;
        ffGetWmiObjUnsigned(pclsObj, L"Chemistry", &chemistry);
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
        ffGetWmiObjUnsigned(pclsObj, L"EstimatedChargeRemaining", &capacity);
        ffStrbufInitF(&battery->capacity, "%d", (int)capacity);

        uint64_t batteryStatus;
        ffGetWmiObjUnsigned(pclsObj, L"BatteryStatus", &batteryStatus);
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

    if(pclsObj) pclsObj->Release();
    pEnumerator->Release();
    return nullptr;
}
