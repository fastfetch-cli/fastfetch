extern "C" {
#include "processes.h"
#include "util/mallocHelper.h"
}

#ifdef FF_USE_WIN_NTAPI

#include <ntstatus.h>
#include <winternl.h>

const char* ffDetectProcesses(uint32_t* result)
{
    ULONG size = 0;
    if(NtQuerySystemInformation(SystemProcessInformation, nullptr, 0, &size) != STATUS_INFO_LENGTH_MISMATCH)
        return "NtQuerySystemInformation(SystemProcessInformation, NULL) failed";

    size += sizeof(SystemProcessInformation) * 5; //What if new processes are created during two syscalls?

    SYSTEM_PROCESS_INFORMATION* FF_AUTO_FREE pstart = (SYSTEM_PROCESS_INFORMATION*)malloc(size);
    if(!pstart)
        return "malloc(size) failed";

    if(!NT_SUCCESS(NtQuerySystemInformation(SystemProcessInformation, pstart, size, nullptr)))
        return "NtQuerySystemInformation(SystemProcessInformation, pstart) failed";

    *result = 1; //Init with 1 because we test for ptr->NextEntryOffset
    for (auto ptr = pstart; ptr->NextEntryOffset; ptr = (SYSTEM_PROCESS_INFORMATION*)((uint8_t*)ptr + ptr->NextEntryOffset))
        ++*result;

    return NULL;
}

#else

#include "util/windows/wmi.hpp"

const char* ffDetectProcesses(uint32_t* result)
{
    FFWmiQuery query(L"SELECT NumberOfProcesses FROM Win32_OperatingSystem", NULL);
    if(!query)
        return "Query WMI service failed";

    if(FFWmiRecord record = query.next())
    {
        uint64_t value = 0;
        record.getUnsigned(L"NumberOfProcesses", &value);
        *result = (uint32_t)value;
        return NULL;
    }

    return "No Wmi result returned";
}

#endif
