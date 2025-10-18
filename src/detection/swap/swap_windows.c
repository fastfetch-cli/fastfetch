#include "swap.h"
#include "util/mallocHelper.h"
#include "util/windows/unicode.h"

#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>

const char* ffDetectSwap(FFlist* result)
{
    uint8_t buffer[4096];
    ULONG size = sizeof(buffer);
    SYSTEM_PAGEFILE_INFORMATION* pstart = (SYSTEM_PAGEFILE_INFORMATION*) buffer;
    if(!NT_SUCCESS(NtQuerySystemInformation(SystemPagefileInformation, pstart, size, &size)))
        return "NtQuerySystemInformation(SystemPagefileInformation, size) failed";

    uint32_t pageSize = instance.state.platform.sysinfo.pageSize;
    for (SYSTEM_PAGEFILE_INFORMATION* current = pstart; ; current = (SYSTEM_PAGEFILE_INFORMATION*)((uint8_t*) current + current->NextEntryOffset))
    {
        FFSwapResult* swap = ffListAdd(result);
        ffStrbufInitNWS(&swap->name, current->FileName.Length / sizeof(wchar_t), current->FileName.Buffer);
        if (ffStrbufStartsWithS(&swap->name, "\\??\\"))
            ffStrbufSubstrAfter(&swap->name, strlen("\\??\\") - 1);
        swap->bytesUsed = (uint64_t) current->TotalUsed * pageSize;
        swap->bytesTotal = (uint64_t) current->CurrentSize * pageSize;
        if (current->NextEntryOffset == 0)
            break;
    }

    return NULL;
}
