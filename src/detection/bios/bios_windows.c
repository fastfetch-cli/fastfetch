#include "bios.h"
#include "util/smbiosHelper.h"
#include "util/windows/registry.h"
#include "efi.h"

#include <ntstatus.h>
#include <winternl.h>
#include <windows.h>

typedef struct _SYSTEM_BOOT_ENVIRONMENT_INFORMATION
{
    GUID BootIdentifier;
    FIRMWARE_TYPE FirmwareType;
    union
    {
        ULONGLONG BootFlags;
        struct
        {
            ULONGLONG DbgMenuOsSelection : 1; // REDSTONE4
            ULONGLONG DbgHiberBoot : 1;
            ULONGLONG DbgSoftBoot : 1;
            ULONGLONG DbgMeasuredLaunch : 1;
            ULONGLONG DbgMeasuredLaunchCapable : 1; // 19H1
            ULONGLONG DbgSystemHiveReplace : 1;
            ULONGLONG DbgMeasuredLaunchSmmProtections : 1;
            ULONGLONG DbgMeasuredLaunchSmmLevel : 7; // 20H1
        };
    };
} SYSTEM_BOOT_ENVIRONMENT_INFORMATION;


typedef struct FFSmbiosBios
{
    FFSmbiosHeader Header;

    uint8_t Vendor; // string
    uint8_t BiosVersion; // string
    uint16_t BiosStartingAddressSegment; // varies
    uint8_t BiosReleaseDate; // string
    uint8_t BiosRomSize; // string
    uint64_t BiosCharacteristics; // bit field

    // 2.4+
    uint8_t BiosCharacteristicsExtensionBytes[2]; // bit field
    uint8_t SystemBiosMajorRelease; // varies
    uint8_t SystemBiosMinorRelease; // varies
    uint8_t EmbeddedControllerFirmwareMajorRelease; // varies
    uint8_t EmbeddedControllerFirmwareMinorRelease; // varies

    // 3.1+
    uint16_t ExtendedBiosRomSize; // bit field
} __attribute__((__packed__)) FFSmbiosBios;

static_assert(offsetof(FFSmbiosBios, ExtendedBiosRomSize) == 0x18,
    "FFSmbiosBios: Wrong struct alignment");

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

const char *detectBootmgr(FFstrbuf *result)
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

    for (
        ffEfiDevicePathProtocol* filePathList = (void*) &efiOption->Description[descLen + 1];
        filePathList->Type != 0x7F; // End of Hardware Device Path
        filePathList = (void*) ((uint8_t*) filePathList + filePathList->Length))
    {
        if (filePathList->Type == 4 && filePathList->SubType == 4)
        {
            // https://uefi.org/specs/UEFI/2.10/10_Protocols_Device_Path_Protocol.html#file-path-media-device-path
            ffUcs2ToUtf8((uint16_t*) filePathList->SpecificDevicePathData, result);
            return NULL;
        }
    }

    if (!result->length) ffUcs2ToUtf8(efiOption->Description, result);
    return NULL;
}

const char *detectSecureBoot(bool* result)
{
    DWORD uefiSecureBootEnabled = 0, bufSize = 0;
    if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SYSTEM\\ControlSet001\\Control\\SecureBoot\\State", L"UEFISecureBootEnabled", RRF_RT_REG_DWORD, NULL, &uefiSecureBootEnabled, &bufSize) != ERROR_SUCCESS)
        return "RegGetValueW(HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\SecureBoot\\State) failed";
    *result = !!uefiSecureBootEnabled;
    return NULL;
}

const char* ffDetectBios(FFBiosResult* bios)
{
    const FFSmbiosHeaderTable* smbiosTable = ffGetSmbiosHeaderTable();
    if (!smbiosTable)
        return "Failed to get SMBIOS data";

    const FFSmbiosBios* data = (const FFSmbiosBios*) (*smbiosTable)[FF_SMBIOS_TYPE_BIOS];
    if (!data)
        return "BIOS section is not found in SMBIOS data";

    const char* strings = (const char*) data + data->Header.Length;

    ffStrbufSetStatic(&bios->version, ffSmbiosLocateString(strings, data->BiosVersion));
    ffCleanUpSmbiosValue(&bios->version);
    ffStrbufSetStatic(&bios->vendor, ffSmbiosLocateString(strings, data->Vendor));
    ffCleanUpSmbiosValue(&bios->vendor);
    ffStrbufSetStatic(&bios->date, ffSmbiosLocateString(strings, data->BiosReleaseDate));
    ffCleanUpSmbiosValue(&bios->date);

    if (data->Header.Length > offsetof(FFSmbiosBios, SystemBiosMajorRelease))
        ffStrbufSetF(&bios->release, "%u.%u", data->SystemBiosMajorRelease, data->SystemBiosMinorRelease);

    // Same as GetFirmwareType, but support (?) Windows 7
    // https://ntdoc.m417z.com/system_information_class
    SYSTEM_BOOT_ENVIRONMENT_INFORMATION sbei;
    if (NT_SUCCESS(NtQuerySystemInformation(90 /*SystemBootEnvironmentInformation*/, &sbei, sizeof(sbei), NULL)))
    {
        switch (sbei.FirmwareType)
        {
            case FirmwareTypeBios: ffStrbufSetStatic(&bios->type, "BIOS"); break;
            case FirmwareTypeUefi: ffStrbufSetStatic(&bios->type, "UEFI"); break;
            default: break;
        }

        if (sbei.FirmwareType == FirmwareTypeUefi)
        {
            detectSecureBoot(&bios->secureBoot);
            detectBootmgr(&bios->bootmgr);
        }
    }

    return NULL;
}
