extern "C" {
#include "cpu.h"
}
#include "util/windows/wmi.hpp"

extern "C"
void ffDetectCPUImpl(const FFinstance* instance, FFCPUResult* cpu, bool cached)
{
    FF_UNUSED(instance);

    cpu->temperature = FF_CPU_TEMP_UNSET;

    if(cached)
        return;

    cpu->coresPhysical = cpu->coresLogical = cpu->coresOnline = 0;
    ffStrbufInit(&cpu->name);
    ffStrbufInit(&cpu->vendor);

    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Name, Manufacturer, NumberOfCores, NumberOfLogicalProcessors, ThreadCount, CurrentClockSpeed, MaxClockSpeed FROM Win32_Processor WHERE ProcessorType = 3", nullptr);
    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    if(FAILED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) || uReturn == 0)
    {
        pEnumerator->Release();
        return;
    }

    ffGetWmiObjString(pclsObj, L"Name", &cpu->name);
    ffGetWmiObjString(pclsObj, L"Manufacturer", &cpu->vendor);

    uint64_t value;

    ffGetWmiObjUnsigned(pclsObj, L"NumberOfCores", &value);
    cpu->coresPhysical = (uint16_t)value;
    ffGetWmiObjUnsigned(pclsObj, L"NumberOfLogicalProcessors", &value);
    cpu->coresLogical = (uint16_t)value;
    ffGetWmiObjUnsigned(pclsObj, L"ThreadCount", &value);
    cpu->coresOnline = (uint16_t)value;
    ffGetWmiObjUnsigned(pclsObj, L"CurrentClockSpeed", &value); //There's no MinClockSpeed in Win32_Processor
    cpu->frequencyMin = (double)value / 1000.0;
    ffGetWmiObjUnsigned(pclsObj, L"MaxClockSpeed", &value);
    cpu->frequencyMax = (double)value / 1000.0;

    pclsObj->Release();
    pEnumerator->Release();
}
