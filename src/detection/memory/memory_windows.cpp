extern "C" {
#include "memory.h"
}
#include "util/windows/wmi.hpp"

extern "C"
void ffDetectMemoryImpl(FFMemoryStorage* ram)
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
