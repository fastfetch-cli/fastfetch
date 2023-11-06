#include "bios.h"
#include "util/windows/registry.h"
#include "util/smbiosHelper.h"

#include <ntstatus.h>
#include <winternl.h>

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

const char* ffDetectBios(FFBiosResult* bios)
{
    FF_HKEY_AUTO_DESTROY hKey = NULL;
    if(!ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", &hKey, NULL))
        return "ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, L\"HARDWARE\\DESCRIPTION\\System\\BIOS\", &hKey, NULL) failed";

    if(!ffRegReadStrbuf(hKey, L"BIOSVersion", &bios->version, NULL))
        return "\"HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\BIOS\\BIOSVersion\" doesn't exist";

    ffCleanUpSmbiosValue(&bios->version);
    ffRegReadStrbuf(hKey, L"BIOSVendor", &bios->vendor, NULL);
    ffCleanUpSmbiosValue(&bios->vendor);
    ffRegReadStrbuf(hKey, L"BIOSReleaseDate", &bios->date, NULL);
    ffCleanUpSmbiosValue(&bios->date);

    uint32_t major, minor;
    if(
        ffRegReadUint(hKey, L"BiosMajorRelease", &major, NULL) &&
        ffRegReadUint(hKey, L"BiosMinorRelease", &minor, NULL)
    )
        ffStrbufAppendF(&bios->release, "%u.%u", (unsigned)major, (unsigned)minor);

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
    }

    return NULL;
}
