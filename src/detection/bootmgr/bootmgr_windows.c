#include "bootmgr.h"
#include "efi.h"

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

    uint16_t value;
    if (GetFirmwareEnvironmentVariableW(L"BootCurrent", L"{" FF_EFI_GLOBAL_GUID L"}", &value, sizeof(value)) != 2)
        return "GetFirmwareEnvironmentVariableW(BootCurrent) failed";

    uint8_t buffer[2048];
    wchar_t key[16];
    wsprintfW(key, L"Boot%04X", value);
    uint32_t size = GetFirmwareEnvironmentVariableW(key, L"{" FF_EFI_GLOBAL_GUID L"}", buffer, sizeof(buffer));
    if (size < sizeof(FFEfiLoadOption) || size == sizeof(buffer))
        return "GetFirmwareEnvironmentVariableW(Boot####) failed";

    FFEfiLoadOption *efiOption = (FFEfiLoadOption *)buffer;
    uint32_t descLen = 0;
    while (efiOption->Description[descLen]) ++descLen;

    if (descLen)
        ffEfiUcs2ToUtf8(efiOption->Description, &result->name);

    for (
        ffEfiDevicePathProtocol* filePathList = (void*) &efiOption->Description[descLen + 1];
        filePathList->Type != 0x7F; // End of Hardware Device Path
        filePathList = (void*) ((uint8_t*) filePathList + filePathList->Length))
    {
        if (filePathList->Type == 4 && filePathList->SubType == 4)
        {
            // https://uefi.org/specs/UEFI/2.10/10_Protocols_Device_Path_Protocol.html#file-path-media-device-path
            ffEfiUcs2ToUtf8((uint16_t*) filePathList->SpecificDevicePathData, &result->firmware);
            break;
        }
    }

    DWORD uefiSecureBootEnabled = 0, bufSize = 0;
    if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Control\\SecureBoot\\State", L"UEFISecureBootEnabled", RRF_RT_REG_DWORD, NULL, &uefiSecureBootEnabled, &bufSize) == ERROR_SUCCESS)
        result->secureBoot = !!uefiSecureBootEnabled;

    return NULL;
}
