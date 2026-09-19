#include "initsystem.h"
#include "common/windows/unicode.h"
#include "common/windows/nt.h"
#include "common/windows/version.h"

#include <ntstatus.h>
#include <winternl.h>
#include <wchar.h>

static bool fillResult(FFInitSystemResult* result, uint32_t ppid, uint32_t pid, uint16_t len, PCWSTR name) {
    if (ppid != 4 /* System */ || len <= 4 || _wcsnicmp(name + len - 4, L".exe", 4) != 0) { // smss.exe
        return false;
    }
    result->pid = pid;
    // We have no permission to open the process for querying the full information
    wchar_t exePath[MAX_PATH];
    _snwprintf(exePath, ARRAY_SIZE(exePath), L"%ls\\system32\\%.*ls", (const wchar_t*) SharedUserData->NtSystemRoot, len, name);
    ffGetFileVersion(exePath, NULL, &result->version);
    ffStrbufSetWS(&result->exe, exePath);
    ffStrbufSetNWS(&result->name, len - 4, name);
    return true;
}

const char* ffDetectInitSystem(FFInitSystemResult* result) {
    if (ffIsSystemBasicProcessInfoAvailable()) {
        // SYSTEM_BASICPROCESS_INFORMATION entries are much smaller than SYSTEM_PROCESS_INFORMATION ones,
        // so a modest buffer should be enough to contain all processes
        SYSTEM_BASICPROCESS_INFORMATION buffer[1024];
        NTSTATUS status = NtQuerySystemInformation(SystemBasicProcessInformation, buffer, sizeof(buffer), NULL);
        if (status != STATUS_INFO_LENGTH_MISMATCH && !NT_SUCCESS(status)) {
            goto fallback;
        }
        for (auto ptr = buffer; ;ptr = (PSYSTEM_BASICPROCESS_INFORMATION) ((uint8_t*) ptr + ptr->NextEntryOffset)) {
            assert(ptr >= buffer && (uint8_t*) ptr < (uint8_t*) buffer + sizeof(buffer));
            if (fillResult(result, (uint32_t)(uintptr_t) ptr->InheritedFromUniqueProcessId,
                                        (uint32_t)(uintptr_t) ptr->UniqueProcessId,
                                        ptr->ImageName.Length / sizeof(*ptr->ImageName.Buffer),
                                        ptr->ImageName.Buffer)) {
                return nullptr;
            }
            // The last process in the list always has a NextEntryOffset of 0, even if the buffer was truncated.
            if (!ptr->NextEntryOffset) {
                return "Could not find init system process";
            }
        }
    }

fallback:
    {
        // We only need to find the first user process, so 1024 entries should be enough
        SYSTEM_PROCESS_INFORMATION buffer[1024] = {};
        ULONG size = sizeof(buffer);
        NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, buffer, size, &size);
        if (status != STATUS_INFO_LENGTH_MISMATCH && !NT_SUCCESS(status)) {
            return "NtQuerySystemInformation(SystemProcessInformation) failed";
        }

        for (auto ptr = buffer; ; ptr = (SYSTEM_PROCESS_INFORMATION*) ((uint8_t*) ptr + ptr->NextEntryOffset)) {
            assert(ptr >= buffer && (uint8_t*) ptr < (uint8_t*) buffer + sizeof(buffer));
            uint16_t len = ptr->ImageName.Length / sizeof(*ptr->ImageName.Buffer);
            if (fillResult(result, (uint32_t)(uintptr_t) ptr->InheritedFromUniqueProcessId,
                                    (uint32_t)(uintptr_t) ptr->UniqueProcessId, len, ptr->ImageName.Buffer)) {
                return nullptr;
            }
            // The last process in the list always has a NextEntryOffset of 0, even if the buffer was truncated.
            if (!ptr->NextEntryOffset) {
                return "Could not find init system process";
            }
        }
    }
}
