extern "C" {
#include "gpu.h"
}
#include "util/windows/wmi.hpp"

extern "C"
const char* ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(instance);

    FFWmiQuery query(L"SELECT Name, AdapterCompatibility, DriverVersion FROM Win32_VideoController", nullptr);
    if(!query)
        return "Query WMI service failed";

    while(FFWmiRecord record = query.next())
    {
        FFGPUResult* gpu = (FFGPUResult*)ffListAdd(gpus);

        ffStrbufInit(&gpu->vendor);
        record.getString(L"AdapterCompatibility", &gpu->vendor);
        if(ffStrbufStartsWithS(&gpu->vendor, "Intel "))
        {
            //Intel returns "Intel Corporation", not sure about AMD
            ffStrbufSetS(&gpu->vendor, "Intel");
        }

        ffStrbufInit(&gpu->name);
        record.getString(L"Name", &gpu->name);

        ffStrbufInit(&gpu->driver);
        record.getString(L"DriverVersion", &gpu->driver);

        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
    }

    return nullptr;
}
