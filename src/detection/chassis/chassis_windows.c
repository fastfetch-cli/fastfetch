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
} FFSmbiosSystemEnclosure;

const char* ffDetectChassis(FFChassisResult* result)
{
    const FFRawSmbiosData* data = ffGetSmbiosData();

    for (
        const FFSmbiosHeader* header = (const FFSmbiosHeader*) data->SMBIOSTableData;
        (const uint8_t*) header < data->SMBIOSTableData + data->Length && header->Type != FF_SMBIOS_TYPE_END_OF_TABLE;
        header = ffSmbiosSkipLastStr(header)
    )
    {
        if (header->Type != FF_SMBIOS_TYPE_SYSTEM_ENCLOSURE)
            continue;

        const FFSmbiosSystemEnclosure* data = (const FFSmbiosSystemEnclosure*) header;
        const char* strings = (const char*) header + header->Length;

        ffStrbufSetStatic(&result->vendor, ffSmbiosLocateString(strings, data->Manufacturer));
        ffCleanUpSmbiosValue(&result->vendor);
        ffStrbufSetStatic(&result->version, ffSmbiosLocateString(strings, data->Version));
        ffCleanUpSmbiosValue(&result->version);
        ffStrbufSetStatic(&result->type, ffChassisTypeToString(data->Type));

        return NULL;
    }

    return "System enclosure is not found in SMBIOS data";
}
