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

    FFWmiQuery query(L"SELECT Name, Manufacturer, NumberOfCores, NumberOfLogicalProcessors, ThreadCount, CurrentClockSpeed, MaxClockSpeed FROM Win32_Processor WHERE ProcessorType = 3");
    if(!query)
        return;

    if(FFWmiRecord record = query.next())
    {
        record.getString(L"Name", &cpu->name);
        record.getString(L"Manufacturer", &cpu->vendor);

        uint64_t value;

        record.getUnsigned(L"NumberOfCores", &value);
        cpu->coresPhysical = (uint16_t)value;
        record.getUnsigned(L"NumberOfLogicalProcessors", &value);
        cpu->coresLogical = (uint16_t)value;
        record.getUnsigned(L"ThreadCount", &value);
        cpu->coresOnline = (uint16_t)value;
        record.getUnsigned(L"CurrentClockSpeed", &value); //There's no MinClockSpeed in Win32_Processor
        cpu->frequencyMin = (double)value / 1000.0;
        record.getUnsigned(L"MaxClockSpeed", &value);
        cpu->frequencyMax = (double)value / 1000.0;
    }
}
