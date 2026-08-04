#include "initsystem.h"
#include "common/windows/unicode.h"
#include "common/windows/nt.h"
#include "common/windows/version.h"

#include <ntstatus.h>
#include <winternl.h>
#include <wchar.h>

const char* ffDetectInitSystem(FFInitSystemResult* result) {
    // We only need to find the first user process, so 1024 entries should be enough
    SYSTEM_PROCESS_INFORMATION buffer[1024] = {};
    ULONG size = sizeof(buffer);
    NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, buffer, size, &size);
    if (status != STATUS_INFO_LENGTH_MISMATCH && !NT_SUCCESS(status)) {
        return "NtQuerySystemInformation(SystemProcessInformation) failed";
    }

    for (SYSTEM_PROCESS_INFORMATION* ptr = buffer; ; ptr = (SYSTEM_PROCESS_INFORMATION*) ((uint8_t*) ptr + ptr->NextEntryOffset)) {
        assert(ptr >= buffer && (uint8_t*) ptr < (uint8_t*) buffer + sizeof(buffer));
        uint16_t len = ptr->ImageName.Length / sizeof(*ptr->ImageName.Buffer);
        if (ptr->InheritedFromUniqueProcessId == (HANDLE)(uintptr_t) 4 /* System */ &&
            len > 4 && _wcsnicmp(ptr->ImageName.Buffer + len - 4, L".exe", 4) == 0) { // smss.exe
            result->pid = (uint32_t)(uintptr_t) ptr->UniqueProcessId;
            // We have no permission to open the process for querying the full information
            wchar_t exePath[MAX_PATH];
            _snwprintf(exePath, ARRAY_SIZE(exePath), L"%ls\\system32\\%.*ls", (const wchar_t*) SharedUserData->NtSystemRoot, len, ptr->ImageName.Buffer);
            ffGetFileVersion(exePath, NULL, &result->version);
            ffStrbufSetWS(&result->exe, exePath);
            ffStrbufSetNWS(&result->name, len - 4, ptr->ImageName.Buffer);
            return nullptr;
        }
        // The last process in the list always has a NextEntryOffset of 0, even if the buffer was truncated.
        if (!ptr->NextEntryOffset) {
            return "Could not find init system process";
        }
    }
}
