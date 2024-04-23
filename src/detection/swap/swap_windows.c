#include "swap.h"
#include "util/mallocHelper.h"

#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    ULONG size = sizeof(SYSTEM_PAGEFILE_INFORMATION) * 2;
    SYSTEM_PAGEFILE_INFORMATION* FF_AUTO_FREE pstart = (SYSTEM_PAGEFILE_INFORMATION*)malloc(size);
    while(true)
    {
        NTSTATUS status = NtQuerySystemInformation(SystemPagefileInformation, pstart, size, &size);
        if(status == STATUS_INFO_LENGTH_MISMATCH)
        {
            if(!(pstart = (SYSTEM_PAGEFILE_INFORMATION*)realloc(pstart, size)))
                return "realloc(pstart, size) failed";
            continue;
        }
        else if(!NT_SUCCESS(status))
            return "NtQuerySystemInformation(SystemPagefileInformation, size) failed";
        break;
    }

    uint32_t pageSize = instance.state.platform.pageSize;
    swap->bytesUsed = (uint64_t)pstart->TotalUsed * pageSize;
    swap->bytesTotal = (uint64_t)pstart->CurrentSize * pageSize;

    return NULL;
}
