#include "swap.h"

#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>

const char* ffDetectSwap(FFSwapResult* swap)
{
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    SYSTEM_PAGEFILE_INFORMATION info;
    if (!NT_SUCCESS(NtQuerySystemInformation(SystemPagefileInformation, &info, sizeof(info), NULL)))
        return "NtQuerySystemInformation(SystemPagefileInformation) failed";
    swap->bytesUsed = (uint64_t)info.TotalUsed * sysInfo.dwPageSize;
    swap->bytesTotal = (uint64_t)info.CurrentSize * sysInfo.dwPageSize;

    return NULL;
}
