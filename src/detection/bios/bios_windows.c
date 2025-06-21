#include "bios.h"
#include "util/smbiosHelper.h"

#ifdef _WIN32
#include "util/windows/registry.h"

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
#elif __OpenBSD__
#include "common/io/io.h"

#include <fcntl.h>
#include <unistd.h>

#elif __sun
#include <libdevinfo.h>
#include <sys/sunddi.h>
#endif

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

    #ifdef _WIN32
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
    #elif __sun
    di_node_t rootNode = di_init("/", DINFOPROP);
    if (rootNode != DI_NODE_NIL)
    {
        char* efiVersion = NULL;
        if (di_prop_lookup_strings(DDI_DEV_T_ANY, rootNode, "efi-version", &efiVersion) > 0)
            ffStrbufSetStatic(&bios->type, "UEFI");
        else
            ffStrbufSetStatic(&bios->type, "BIOS");
    }
    di_fini(rootNode);
    #elif __HAIKU__ || __OpenBSD__
    // Currently SMBIOS detection is supported in legancy BIOS only
    ffStrbufSetStatic(&bios->type, "BIOS");
    #endif

    return NULL;
}
