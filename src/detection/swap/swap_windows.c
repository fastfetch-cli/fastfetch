#include "swap.h"
#include "util/mallocHelper.h"

#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>

const char* ffDetectSwap(FFSwapResult* swap)
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
                return "realloc(pstart, size) failed";
        }
        else if(!NT_SUCCESS(status))
            return "NtQuerySystemInformation(SystemPagefileInformation, size) failed";
        break;
    }
    swap->bytesUsed = (uint64_t)pstart->TotalUsed * sysInfo.dwPageSize;
    swap->bytesTotal = (uint64_t)pstart->CurrentSize * sysInfo.dwPageSize;
}
