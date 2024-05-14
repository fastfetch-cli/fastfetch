#include "board.h"
#include "util/smbiosHelper.h"

typedef struct FFSmbiosBaseboard
{
    FFSmbiosHeader Header;

    uint8_t Manufacturer; // string
    uint8_t Product; // string
    uint8_t Version; // string
    uint8_t SerialNumber; // string
    uint8_t AssetTag; // string
    uint8_t FeatureFlags; // bit field
    uint8_t LocationInChassis; // string
    uint16_t ChassisHandle; // varies
    uint8_t BoardType; // enum
    uint8_t NumberOfContainedObjectHandles; // varies
    uint16_t ContainedObjectHandles[]; // varies
} __attribute__((__packed__)) FFSmbiosBaseboard;

static_assert(offsetof(FFSmbiosBaseboard, ContainedObjectHandles) == 0x0F,
    "FFSmbiosBaseboard: Wrong struct alignment");

const char* ffDetectBoard(FFBoardResult* board)
{
    const FFSmbiosHeaderTable* smbiosTable = ffGetSmbiosHeaderTable();
    if (!smbiosTable)
        return "Failed to get SMBIOS data";

    const FFSmbiosBaseboard* data = (const FFSmbiosBaseboard*) (*smbiosTable)[FF_SMBIOS_TYPE_BASEBOARD_INFO];
    if (!data)
        return "Baseboard information section is not found in SMBIOS data";

    const char* strings = (const char*) data + data->Header.Length;

    ffStrbufSetStatic(&board->name, ffSmbiosLocateString(strings, data->Product));
    ffCleanUpSmbiosValue(&board->name);
    ffStrbufSetStatic(&board->serial, ffSmbiosLocateString(strings, data->SerialNumber));
    ffCleanUpSmbiosValue(&board->serial);
    ffStrbufSetStatic(&board->vendor, ffSmbiosLocateString(strings, data->Manufacturer));
    ffCleanUpSmbiosValue(&board->vendor);
    ffStrbufSetStatic(&board->version, ffSmbiosLocateString(strings, data->Version));
    ffCleanUpSmbiosValue(&board->version);

    return NULL;
}
