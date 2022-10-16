extern "C" {
#include "memory.h"
}
#include "util/windows/wmi.hpp"

void detectRam(FFMemoryStorage* ram)
{
    FFWmiQuery query(L"SELECT TotalVisibleMemorySize, FreePhysicalMemory FROM Win32_OperatingSystem", &ram->error);
    if(!query)
        return;

    if(FFWmiRecord record = query.next())
    {
        //KB
        record.getUnsigned(L"TotalVisibleMemorySize", &ram->bytesTotal);
        uint64_t bytesFree;
        record.getUnsigned(L"FreePhysicalMemory", &bytesFree);
        ram->bytesUsed = ram->bytesTotal - bytesFree;
        ram->bytesTotal *= 1024;
        ram->bytesUsed *= 1024;
    }
    else
        ffStrbufInitS(&ram->error, "No Wmi result returned");
}

void detectSwap(FFMemoryStorage* swap)
{
    FFWmiQuery query(L"SELECT AllocatedBaseSize, CurrentUsage FROM Win32_PageFileUsage", &swap->error);
    if(!query)
        return;

    if(FFWmiRecord record = query.next())
    {
        //MB
        record.getUnsigned(L"AllocatedBaseSize", &swap->bytesTotal);
        record.getUnsigned(L"CurrentUsage", &swap->bytesUsed);
        swap->bytesTotal *= 1024 * 1024;
        swap->bytesUsed *= 1024 * 1024;
    }
    else
        ffStrbufInitS(&swap->error, "No Wmi result returned");
}

extern "C"
void ffDetectMemoryImpl(FFMemoryResult* memory)
{
    detectRam(&memory->ram);
    detectSwap(&memory->swap);
}
