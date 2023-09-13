#include "processes.h"
#include "util/mallocHelper.h"

#include <ntstatus.h>
#include <winternl.h>

const char* ffDetectProcesses(uint32_t* result)
{
    SYSTEM_PROCESS_INFORMATION* FF_AUTO_FREE pstart = NULL;

    // Multiple attempts in case processes change while
    // we are in the middle of querying them.
    ULONG size = 0;
    for (int attempts = 0;; ++attempts)
    {
        if (size)
        {
            pstart = (SYSTEM_PROCESS_INFORMATION*)realloc(pstart, size);
            assert(pstart);
        }
        NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, pstart, size, &size);
        if(NT_SUCCESS(status))
            break;
        else if(status == STATUS_INFO_LENGTH_MISMATCH && attempts < 4)
            size += sizeof(SYSTEM_PROCESS_INFORMATION) * 5;
        else
            return "NtQuerySystemInformation(SystemProcessInformation) failed";
    }

    *result = 1; //Init with 1 because we test for ptr->NextEntryOffset
    for (SYSTEM_PROCESS_INFORMATION* ptr = pstart; ptr->NextEntryOffset; ptr = (SYSTEM_PROCESS_INFORMATION*)((uint8_t*)ptr + ptr->NextEntryOffset))
        ++*result;

    return NULL;
}
