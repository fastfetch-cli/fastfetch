#include "bootmgr.h"
#include "efi_helper.h"

#include <windows.h>

const char* enablePrivilege(const wchar_t* privilege)
{
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
        return "OpenProcessToken() failed";

    TOKEN_PRIVILEGES tp = {
        .PrivilegeCount = 1,
        .Privileges = {
            (LUID_AND_ATTRIBUTES) { .Attributes = SE_PRIVILEGE_ENABLED }
        },
    };
    if (!LookupPrivilegeValueW(NULL, privilege, &tp.Privileges[0].Luid))
        return "LookupPrivilegeValue() failed";

    if (!AdjustTokenPrivileges(token, false, &tp, sizeof(tp), NULL, NULL))
        return "AdjustTokenPrivileges() failed";

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
        return "The token does not have the specified privilege";

    return NULL;
}

const char* ffDetectBootmgr(FFBootmgrResult* result)
{
    if (enablePrivilege(L"SeSystemEnvironmentPrivilege") != NULL)
        return "Failed to enable SeSystemEnvironmentPrivilege";

    if (GetFirmwareEnvironmentVariableW(L"BootCurrent", L"{" FF_EFI_GLOBAL_GUID L"}", &result->order, sizeof(result->order)) != 2)
        return "GetFirmwareEnvironmentVariableW(BootCurrent) failed";

    uint8_t buffer[2048];
    wchar_t key[16];
    swprintf(key, ARRAY_SIZE(key), L"Boot%04X", result->order);
    uint32_t size = GetFirmwareEnvironmentVariableW(key, L"{" FF_EFI_GLOBAL_GUID L"}", buffer, sizeof(buffer));
    if (size < sizeof(FFEfiLoadOption) || size == ARRAY_SIZE(buffer))
        return "GetFirmwareEnvironmentVariableW(Boot####) failed";

    ffEfiFillLoadOption((FFEfiLoadOption *)buffer, result);

    DWORD uefiSecureBootEnabled = 0, bufSize = 0;
    if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State", L"UEFISecureBootEnabled", RRF_RT_REG_DWORD, NULL, &uefiSecureBootEnabled, &bufSize) == ERROR_SUCCESS)
        result->secureBoot = !!uefiSecureBootEnabled;

    return NULL;
}
