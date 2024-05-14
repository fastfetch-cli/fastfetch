#include "chassis.h"
#include "util/smbiosHelper.h"

// 7.4
typedef struct FFSmbiosSystemEnclosure
{
    FFSmbiosHeader Header;

    uint8_t Manufacturer; // string
    uint8_t Type; // varies
    uint8_t Version; // string
    uint8_t SerialNumber; // string
    uint8_t AssetTagNumber; // string

    // 2.1+
    uint8_t BootupState; // enum
    uint8_t PowerSupplyState; // enum
    uint8_t ThermalState; // enum
    uint8_t SecurityStatus; // enum

    // 2.3+
    uint32_t OEMDefined; // varies
    uint8_t Height; // varies
    uint8_t NumberOfPowerCords; // varies
    uint8_t ContainedElementCount; // varies
    uint8_t ContainedRecordLength; // varies
    uint8_t ContainedElements[]; // varies
} __attribute__((__packed__)) FFSmbiosSystemEnclosure;

static_assert(offsetof(FFSmbiosSystemEnclosure, ContainedElements) == 0x15,
    "FFSmbiosSystemEnclosure: Wrong struct alignment");

const char* ffDetectChassis(FFChassisResult* result)
{
    const FFSmbiosHeaderTable* smbiosTable = ffGetSmbiosHeaderTable();
    if (!smbiosTable)
        return "Failed to get SMBIOS data";

    const FFSmbiosSystemEnclosure* data = (const FFSmbiosSystemEnclosure*) (*smbiosTable)[FF_SMBIOS_TYPE_SYSTEM_ENCLOSURE];
    if (!data)
        return "System enclosure is not found in SMBIOS data";

    const char* strings = (const char*) data + data->Header.Length;

    ffStrbufSetStatic(&result->vendor, ffSmbiosLocateString(strings, data->Manufacturer));
    ffCleanUpSmbiosValue(&result->vendor);
    ffStrbufSetStatic(&result->serial, ffSmbiosLocateString(strings, data->SerialNumber));
    ffCleanUpSmbiosValue(&result->serial);
    ffStrbufSetStatic(&result->version, ffSmbiosLocateString(strings, data->Version));
    ffCleanUpSmbiosValue(&result->version);
    ffStrbufSetStatic(&result->type, ffChassisTypeToString(data->Type));

    return NULL;
}
