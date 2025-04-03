#include "swap.h"
#include "util/mallocHelper.h"

#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    uint8_t buffer[4096];
    ULONG size = sizeof(buffer);
    SYSTEM_PAGEFILE_INFORMATION* pstart = (SYSTEM_PAGEFILE_INFORMATION*) buffer;
    if(!NT_SUCCESS(NtQuerySystemInformation(SystemPagefileInformation, pstart, size, &size)))
        return "NtQuerySystemInformation(SystemPagefileInformation, size) failed";

    for (SYSTEM_PAGEFILE_INFORMATION* current = pstart; ; current = (SYSTEM_PAGEFILE_INFORMATION*)((uint8_t*) current + current->NextEntryOffset))
    {
        swap->bytesUsed += current->TotalUsed;
        swap->bytesTotal += current->CurrentSize;
        if (current->NextEntryOffset == 0)
            break;
    }
    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;
    swap->bytesUsed *= pageSize;
    swap->bytesTotal *= pageSize;

    return NULL;
}
