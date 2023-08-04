#include "memory.h"

#include <winternl.h>
#include <ntstatus.h>
#include <windows.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    SYSTEM_PERFORMANCE_INFORMATION info;
    if (!NT_SUCCESS(NtQuerySystemInformation(SystemPerformanceInformation, &info, sizeof(info), NULL)))
        return "NtQuerySystemInformation(SystemPerformanceInformation) failed";
    ram->bytesUsed = (uint64_t)(info.TotalCommitLimit - info.AvailablePages) * sysInfo.dwPageSize;
    ram->bytesTotal = (uint64_t)info.TotalCommitLimit * sysInfo.dwPageSize;

    return NULL;
}
