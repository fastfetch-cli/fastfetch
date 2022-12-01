extern "C" {
#include "swap.h"
#include "util/mallocHelper.h"
}

#ifdef FF_USE_WIN_NTAPI

#include <winternl.h>
#include <ntstatus.h>

extern "C"
void ffDetectSwapImpl(FFMemoryStorage* swap)
{
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    ULONG size = sizeof(SYSTEM_PAGEFILE_INFORMATION);
    SYSTEM_PAGEFILE_INFORMATION* FF_AUTO_FREE pstart = (SYSTEM_PAGEFILE_INFORMATION*)malloc(size);
    while(true)
    {
        NTSTATUS status = NtQuerySystemInformation(SystemPagefileInformation, pstart, size, &size);
        if(status == STATUS_INFO_LENGTH_MISMATCH)
        {
            if(!(pstart = (SYSTEM_PAGEFILE_INFORMATION*)realloc(pstart, size)))
            {
                ffStrbufAppendF(&swap->error, "relloc(pstart, %lu) failed", size);
                return;
            }
        }
        else if(!NT_SUCCESS(status))
        {
            ffStrbufAppendF(&swap->error, "NtQuerySystemInformation(SystemPagefileInformation, %lu) failed", size);
            return;
        }
        break;
    }
    swap->bytesUsed = (uint64_t)pstart->TotalUsed * sysInfo.dwPageSize;
    swap->bytesTotal = (uint64_t)pstart->CurrentSize * sysInfo.dwPageSize;
}

#else

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

#endif
