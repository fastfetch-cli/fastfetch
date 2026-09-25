#include "processes.h"
#include "common/mallocHelper.h"
#include "common/windows/nt.h"

#include <ntstatus.h>
#include <winternl.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    if (options->countKprocs && ffIsSystemBasicProcessInfoAvailable()) {
        // SystemHandleCountInformation reports the total process/thread counts directly,
        // so we don't need to walk the whole process table.
        SYSTEM_HANDLECOUNT_INFORMATION info = {}; // Seems that kernel only fills the lower 32 bits of the counts, leave the upper 32 bits untouched.
        if (NT_SUCCESS(NtQuerySystemInformation(SystemHandleCountInformation, &info, sizeof(info), NULL))) {
            result->processes = info.ProcessCount;
            result->threads = info.ThreadCount;
            return nullptr;
        }
        // Otherwise fall back to walking the process table
    }

    FF_AUTO_FREE SYSTEM_PROCESS_INFORMATION* pstart = nullptr;

    // Multiple attempts in case processes change while
    // we are in the middle of querying them.
    ULONG size = 0;
    for (int attempts = 0;; ++attempts) {
        if (size) {
            pstart = (SYSTEM_PROCESS_INFORMATION*) realloc(pstart, size);
            assert(pstart);
        }
        NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, pstart, size, &size);
        if (NT_SUCCESS(status)) {
            break;
        } else if (status == STATUS_INFO_LENGTH_MISMATCH && attempts < 4) {
            size += sizeof(SYSTEM_PROCESS_INFORMATION) * 5;
        } else {
            return "NtQuerySystemInformation(SystemProcessInformation) failed";
        }
    }

    for (SYSTEM_PROCESS_INFORMATION* ptr = pstart; ; ptr = (SYSTEM_PROCESS_INFORMATION*) ((uint8_t*) ptr + ptr->NextEntryOffset)) {
        if (options->countKprocs || (uintptr_t) ptr->InheritedFromUniqueProcessId > 0) {
            ++result->processes;
            result->threads += ptr->NumberOfThreads;
        }
        if (ptr->NextEntryOffset == 0) {
            break;
        }
    }

    return nullptr;
}
