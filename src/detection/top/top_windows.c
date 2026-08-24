#include "top.h"

#include "common/mallocHelper.h"
#include "common/windows/unicode.h"

#include <ntstatus.h>
#include <windows.h>
#include <winternl.h>

const char* ffTopGetProcessSnapshot(FFlist* snapshots) {
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

    for (SYSTEM_PROCESS_INFORMATION* info = pstart;; info = (SYSTEM_PROCESS_INFORMATION*) ((uint8_t*) info + info->NextEntryOffset)) {
        uintptr_t pidValue = (uintptr_t) info->UniqueProcessId;
        if (pidValue <= UINT32_MAX && info->CreateTime.QuadPart > 0) {
            FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);
            item->pid = (uint32_t) pidValue;
            item->startTime = info->CreateTime.QuadPart > 0 ? (uint64_t) info->CreateTime.QuadPart : 0;
            item->cpuTime = ((uint64_t) info->UserTime.QuadPart + (uint64_t) info->KernelTime.QuadPart) / 10000u;
            item->memBytes = (uint64_t) info->VirtualMemoryCounters.WorkingSetSize;
            item->bytesRead = (uint64_t) info->IoCounters.ReadTransferCount;
            item->bytesWritten = (uint64_t) info->IoCounters.WriteTransferCount;
            ffStrbufInitNWS(&item->name, info->ImageName.Length / sizeof(wchar_t), info->ImageName.Buffer);
        }
        if (info->NextEntryOffset == 0) {
            break;
        }
    }
    return nullptr;
}
