#include "swap.h"
#include "util/mallocHelper.h"

#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    uint8_t buffer[1024];
    ULONG size = sizeof(buffer);
    SYSTEM_PAGEFILE_INFORMATION* pstart = (SYSTEM_PAGEFILE_INFORMATION*) buffer;
    if(!NT_SUCCESS(NtQuerySystemInformation(SystemPagefileInformation, pstart, size, &size)))
        return "NtQuerySystemInformation(SystemPagefileInformation, size) failed";

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;
    swap->bytesUsed = (uint64_t)pstart->TotalUsed * pageSize;
    swap->bytesTotal = (uint64_t)pstart->CurrentSize * pageSize;

    return NULL;
}
