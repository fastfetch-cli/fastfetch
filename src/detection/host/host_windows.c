#include "host.h"
#include "util/smbiosHelper.h"

typedef struct FFSmbiosSystemInfo
{
    FFSmbiosHeader Header;

    uint8_t Manufacturer; // string
    uint8_t ProductName; // string
    uint8_t Version; // string
    uint8_t SerialNumber; // string

    // 2.1+
    struct {
        uint32_t TimeLow;
        uint16_t TimeMid;
        uint16_t TimeHighAndVersion;
        uint8_t ClockSeqHiAndReserved;
        uint8_t ClockSeqLow;
        uint8_t Node[6];
    } UUID; // varies
    uint8_t WakeUpType; // enum

    // 2.4+
    uint8_t SKUNumber; // string
    uint8_t Family; // string
} FFSmbiosSystemInfo;

const char* ffDetectHost(FFHostResult* host)
{
    const FFRawSmbiosData* fullData = ffGetSmbiosData();

    for (
        const FFSmbiosHeader* header = (const FFSmbiosHeader*) fullData->SMBIOSTableData;
        (const uint8_t*) header < fullData->SMBIOSTableData + fullData->Length && header->Type != FF_SMBIOS_TYPE_END_OF_TABLE;
        header = ffSmbiosSkipLastStr(header)
    )
    {
        if (header->Type != FF_SMBIOS_TYPE_SYSTEM_INFO)
            continue;

        const FFSmbiosSystemInfo* data = (const FFSmbiosSystemInfo*) header;
        const char* strings = (const char*) header + header->Length;

        ffStrbufSetStatic(&host->vendor, ffSmbiosLocateString(strings, data->Manufacturer));
        ffCleanUpSmbiosValue(&host->vendor);
        ffStrbufSetStatic(&host->name, ffSmbiosLocateString(strings, data->ProductName));
        ffCleanUpSmbiosValue(&host->name);
        ffStrbufSetStatic(&host->version, ffSmbiosLocateString(strings, data->Version));
        ffCleanUpSmbiosValue(&host->version);
        ffStrbufSetStatic(&host->serial, ffSmbiosLocateString(strings, data->SerialNumber));
        ffCleanUpSmbiosValue(&host->serial);

        static_assert(offsetof(FFSmbiosSystemInfo, UUID) == 0x08, "FFSmbiosSystemInfo.UUID offset is wrong");
        if (header->Length > offsetof(FFSmbiosSystemInfo, UUID))
        {
            ffStrbufSetF(&host->uuid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                data->UUID.TimeLow, data->UUID.TimeMid, data->UUID.TimeHighAndVersion,
                data->UUID.ClockSeqHiAndReserved, data->UUID.ClockSeqLow,
                data->UUID.Node[0], data->UUID.Node[1], data->UUID.Node[2], data->UUID.Node[3], data->UUID.Node[4], data->UUID.Node[5]);
        }

        static_assert(offsetof(FFSmbiosSystemInfo, SKUNumber) == 0x19, "FFSmbiosSystemInfo.SKUNumber offset is wrong");
        if (header->Length > offsetof(FFSmbiosSystemInfo, SKUNumber))
        {
            ffStrbufSetStatic(&host->family, ffSmbiosLocateString(strings, data->Family));
            ffCleanUpSmbiosValue(&host->family);
            ffStrbufSetStatic(&host->sku, ffSmbiosLocateString(strings, data->SKUNumber));
            ffCleanUpSmbiosValue(&host->sku);
        }

        return NULL;
    }

    return "System information is not found in SMBIOS data";
}
