extern "C" {
#include "swap.h"
}
#include "util/windows/wmi.hpp"

extern "C"
void ffDetectSwapImpl(FFMemoryStorage* swap)
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
