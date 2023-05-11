#include "processes.h"
#include "util/mallocHelper.h"

#include <ntstatus.h>
#include <winternl.h>

const char* ffDetectProcesses(uint32_t* result)
{
    ULONG size = 0;
    if(NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &size) != STATUS_INFO_LENGTH_MISMATCH)
        return "NtQuerySystemInformation(SystemProcessInformation, NULL) failed";

    size += sizeof(SystemProcessInformation) * 5; //What if new processes are created during two syscalls?

    SYSTEM_PROCESS_INFORMATION* FF_AUTO_FREE pstart = (SYSTEM_PROCESS_INFORMATION*)malloc(size);
    if(!pstart)
        return "malloc(size) failed";

    if(!NT_SUCCESS(NtQuerySystemInformation(SystemProcessInformation, pstart, size, NULL)))
        return "NtQuerySystemInformation(SystemProcessInformation, pstart) failed";

    *result = 1; //Init with 1 because we test for ptr->NextEntryOffset
    for (SYSTEM_PROCESS_INFORMATION* ptr = pstart; ptr->NextEntryOffset; ptr = (SYSTEM_PROCESS_INFORMATION*)((uint8_t*)ptr + ptr->NextEntryOffset))
        ++*result;

    return NULL;
}
