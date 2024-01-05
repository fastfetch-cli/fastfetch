#include "chassis.h"
#include "util/smbiosHelper.h"

// 7.4
typedef struct FFSmbiosSystemEnclosure
{
    uint8_t Manufacturer;
    uint8_t Type;
    uint8_t Version;
    uint8_t SerialNumber;
    uint8_t AssetTagNumber;
    uint8_t BootupState;
    uint8_t PowerSupplyState;
    uint8_t ThermalState;
    uint8_t SecurityStatus;
    uint32_t OEMDefined;
    uint8_t Height;
    uint8_t NumberOfPowerCords;
} FFSmbiosSystemEnclosure;

const char* ffDetectChassis(FFChassisResult* result, FFChassisOptions* options)
{
    const FFRawSmbiosData* data = ffGetSmbiosData();

    for (
        const FFSmbiosHeader* header = (const FFSmbiosHeader*) data->SMBIOSTableData;
        (const uint8_t*) header < data->SMBIOSTableData + data->Length && header->Type < FF_SMBIOS_TYPE_LAST;
        header = ffSmbiosSkipLastStr(header)
    )
    {
        if (header->Type != FF_SMBIOS_TYPE_SYSTEM_ENCLOSURE)
            continue;

        const FFSmbiosSystemEnclosure* data = (const FFSmbiosSystemEnclosure*) header->Data;
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
