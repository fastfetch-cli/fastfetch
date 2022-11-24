extern "C" {
#include "processes.h"
#include "util/mallocHelper.h"
}

#ifdef FF_USE_WIN_NTAPI

#include <winternl.h>

uint32_t ffDetectProcesses(FFinstance* instance, FFstrbuf* error)
{
    FF_UNUSED(instance);

    ULONG size = 0;
    if(NtQuerySystemInformation(SystemProcessInformation, nullptr, 0, &size) != (NTSTATUS)0xC0000004 /*STATUS_INFO_LENGTH_MISMATCH*/)
    {
        ffStrbufAppendS(error, "NtQuerySystemInformation(SystemProcessInformation, NULL) failed");
        return 0;
    }
    size += sizeof(SystemProcessInformation) * 5; //What if new processes are created during two syscalls?

    SYSTEM_PROCESS_INFORMATION* FF_AUTO_FREE pstart = (SYSTEM_PROCESS_INFORMATION*)malloc(size);
    if(!pstart)
    {
        ffStrbufAppendF(error, "malloc(%u) failed", (unsigned)size);
        return 0;
    }

    if(!NT_SUCCESS(NtQuerySystemInformation(SystemProcessInformation, pstart, size, nullptr)))
    {
        ffStrbufAppendS(error, "NtQuerySystemInformation(SystemProcessInformation, pstart) failed");
        return 0;
    }

    uint32_t result = 1; //Init with 1 because we test for ptr->NextEntryOffset
    for (auto ptr = pstart; ptr->NextEntryOffset; ptr = (SYSTEM_PROCESS_INFORMATION*)((uint8_t*)ptr + ptr->NextEntryOffset))
        ++result;

    return result;
}

#else

#include "util/windows/wmi.hpp"

uint32_t ffDetectProcesses(FFinstance* instance, FFstrbuf* error)
{
    FFWmiQuery query(L"SELECT NumberOfProcesses FROM Win32_OperatingSystem", error);
    if(!query)
        return 0;

    if(FFWmiRecord record = query.next())
    {
        uint64_t result = 0;
        record.getUnsigned(L"NumberOfProcesses", &result);
        return (uint32_t)result;
    }
    else
    {
        ffStrbufAppendS(error, "No Wmi result returned");
        return 0;
    }
}

#endif
