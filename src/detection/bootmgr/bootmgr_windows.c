#include "bootmgr.h"
#include "efi_helper.h"
#include "common/io.h"
#include "common/windows/nt.h"

#include <ntstatus.h>
#include <windows.h>

const char* enablePrivilege(const wchar_t* privilege) {
    FF_AUTO_CLOSE_FD HANDLE token = NULL;
    if (!NT_SUCCESS(NtOpenProcessToken(NtCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))) {
        return "NtOpenProcessToken() failed";
    }

    TOKEN_PRIVILEGES tp = {
        .PrivilegeCount = 1,
        .Privileges = {
            (LUID_AND_ATTRIBUTES) {.Attributes = SE_PRIVILEGE_ENABLED}},
    };
    if (!LookupPrivilegeValueW(NULL, privilege, &tp.Privileges[0].Luid)) {
        return "LookupPrivilegeValue() failed";
    }

    NTSTATUS status = NtAdjustPrivilegesToken(token, false, &tp, sizeof(tp), NULL, NULL);
    if (!NT_SUCCESS(status)) {
        return "NtAdjustPrivilegesToken() failed";
    }

    if (status == STATUS_NOT_ALL_ASSIGNED) {
        return "The token does not have the specified privilege; try sudo please";
    }

    return NULL;
}

const char* ffDetectBootmgr(FFBootmgrResult* result) {
    const char* err = enablePrivilege(L"SeSystemEnvironmentPrivilege");
    if (err != NULL) {
        return err;
    }

    GUID efiGlobalGuid;
    if (!NT_SUCCESS(RtlGUIDFromString(&(UNICODE_STRING) RTL_CONSTANT_STRING(L"{" FF_EFI_GLOBAL_GUID L"}"), &efiGlobalGuid))) {
        return "RtlGUIDFromString() failed";
    }

    ULONG size = sizeof(result->order);
    if (!NT_SUCCESS(NtQuerySystemEnvironmentValueEx(&(UNICODE_STRING) RTL_CONSTANT_STRING(L"BootCurrent"), &efiGlobalGuid, &result->order, &size, NULL))) {
        return "NtQuerySystemEnvironmentValueEx(BootCurrent) failed";
    }
    if (size != sizeof(result->order)) {
        return "NtQuerySystemEnvironmentValueEx(BootCurrent) returned unexpected size";
    }

    uint8_t buffer[2048];
    wchar_t key[9];
    swprintf(key, ARRAY_SIZE(key), L"Boot%04X", result->order);
    size = sizeof(buffer);
    if (!NT_SUCCESS(NtQuerySystemEnvironmentValueEx(&(UNICODE_STRING) RTL_CONSTANT_STRING(key), &efiGlobalGuid, buffer, &size, NULL))) {
        return "NtQuerySystemEnvironmentValueEx(Boot####) failed";
    }
    if (size < sizeof(FFEfiLoadOption) || size == ARRAY_SIZE(buffer)) {
        return "NtQuerySystemEnvironmentValueEx(Boot####) returned unexpected size";
    }

    ffEfiFillLoadOption((FFEfiLoadOption*) buffer, result);

    SYSTEM_SECUREBOOT_INFORMATION ssi;
    if (NT_SUCCESS(NtQuerySystemInformation(SystemSecureBootInformation, &ssi, sizeof(ssi), NULL))) {
        result->secureBoot = ssi.SecureBootEnabled;
    }

    return NULL;
}
