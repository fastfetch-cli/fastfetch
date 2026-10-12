#include "bootmgr.h"
#include "common/debug.h"
#include "efi_helper.h"
#include "common/io.h"
#include "common/windows/nt.h"

#include <ntstatus.h>
#include <windows.h>

const char* enablePrivilege(const wchar_t* privilege) {
    FF_AUTO_CLOSE_FD HANDLE token = nullptr;
    NTSTATUS status = NtOpenProcessToken(NtCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("NtOpenProcessToken() failed: %s", ffDebugNtStatus(status));
        return "NtOpenProcessToken() failed";
    }

    TOKEN_PRIVILEGES tp = {
        .PrivilegeCount = 1,
        .Privileges = {
            (LUID_AND_ATTRIBUTES) { .Attributes = SE_PRIVILEGE_ENABLED } },
    };
    if (!LookupPrivilegeValueW(nullptr, privilege, &tp.Privileges[0].Luid)) {
        FF_DEBUG("LookupPrivilegeValue() failed: %s", ffDebugWin32Error(GetLastError()));
        return "LookupPrivilegeValue() failed";
    }

    status = NtAdjustPrivilegesToken(token, false, &tp, sizeof(tp), nullptr, nullptr);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("NtAdjustPrivilegesToken() failed: %s", ffDebugNtStatus(status));
        return "NtAdjustPrivilegesToken() failed";
    }

    if (status == STATUS_NOT_ALL_ASSIGNED) {
        FF_DEBUG("NtAdjustPrivilegesToken() did not assign the privilege: %s", ffDebugNtStatus(status));
        return "The token does not have the specified privilege; try sudo please";
    }

    return nullptr;
}

const char* ffDetectBootmgr(FFBootmgrResult* result) {
    const char* err = enablePrivilege(L"SeSystemEnvironmentPrivilege");
    if (err != nullptr) {
        return err;
    }

    GUID efiGlobalGuid = {
        .Data1 = 0x8be4df61,
        .Data2 = 0x93ca,
        .Data3 = 0x11d2,
        .Data4 = { 0xaa, 0x0d, 0x00, 0xe0, 0x98, 0x03, 0x2b, 0x8c },
    };

    ULONG size = sizeof(result->order);
    NTSTATUS status = NtQuerySystemEnvironmentValueEx(&(UNICODE_STRING) RTL_CONSTANT_STRING(L"BootCurrent"), &efiGlobalGuid, &result->order, &size, nullptr);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("NtQuerySystemEnvironmentValueEx(BootCurrent) failed: %s", ffDebugNtStatus(status));
        return "NtQuerySystemEnvironmentValueEx(BootCurrent) failed";
    }
    if (size != sizeof(result->order)) {
        FF_DEBUG("NtQuerySystemEnvironmentValueEx(BootCurrent) returned unexpected size");
        return "NtQuerySystemEnvironmentValueEx(BootCurrent) returned unexpected size";
    }

    uint8_t buffer[2048];
    wchar_t key[9];
    swprintf(key, ARRAY_SIZE(key), L"Boot%04X", result->order);
    size = sizeof(buffer);
    status = NtQuerySystemEnvironmentValueEx(&(UNICODE_STRING) RTL_CONSTANT_STRING(key), &efiGlobalGuid, buffer, &size, nullptr);
    if (!NT_SUCCESS(status)) {
        FF_DEBUG("NtQuerySystemEnvironmentValueEx(Boot####) failed: %s", ffDebugNtStatus(status));
        return "NtQuerySystemEnvironmentValueEx(Boot####) failed";
    }
    if (size < sizeof(FFEfiLoadOption) || size == ARRAY_SIZE(buffer)) {
        FF_DEBUG("NtQuerySystemEnvironmentValueEx(Boot####) returned unexpected size");
        return "NtQuerySystemEnvironmentValueEx(Boot####) returned unexpected size";
    }

    ffEfiFillLoadOption((FFEfiLoadOption*) buffer, result);

    SYSTEM_SECUREBOOT_INFORMATION ssi;
    if (NT_SUCCESS(NtQuerySystemInformation(SystemSecureBootInformation, &ssi, sizeof(ssi), nullptr))) {
        result->secureBoot = ssi.SecureBootEnabled;
    }

    return nullptr;
}
