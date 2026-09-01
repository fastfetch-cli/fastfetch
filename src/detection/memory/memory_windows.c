#include "memory.h"

#include "common/windows/nt.h"

const char* ffDetectMemory(FFMemoryResult* ram) {
    // Note: GlobalMemoryStatusEx() internally uses SystemMemoryUsageInformation in Win 10

    SYSTEM_BASIC_PERFORMANCE_INFORMATION sbpi;
    if (!NT_SUCCESS(NtQuerySystemInformation(SystemBasicPerformanceInformation, &sbpi, sizeof(sbpi), NULL)))
        return "Failed to query memory information";

    uint64_t phyPages =
    #if _WIN64
        SharedUserData->FullNumberOfPhysicalPages;
    if (__builtin_expect(phyPages == 0, false)) { // Not supported on Windows 8.1
        phyPages = SharedUserData->NumberOfPhysicalPages; // ULONG, maximum 16 TB
    }
    #else
        SharedUserData->NumberOfPhysicalPages;
    #endif

    uint64_t pageSize = instance.state.platform.sysinfo.pageSize;
    ram->bytesTotal = phyPages * pageSize;
    ram->bytesUsed = (phyPages - sbpi.AvailablePages) * pageSize;
    return nullptr;
}
