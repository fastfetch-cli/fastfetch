#include "physicalmemory.h"
#include "util/smbiosHelper.h"

// 7.18
typedef struct FFSmbiosMemoryDevice
{
    FFSmbiosHeader Header;

    // 2.1+
    uint16_t PhysicalMemoryArrayHandle; // varies
    uint16_t MemoryErrorInformationHandle; //varies
    uint16_t TotalWidth; // varies
    uint16_t DataWidth; // varies
    uint16_t Size; // varies
    uint8_t FormFactor; // enum
    uint8_t DeviceSet; // varies
    uint8_t DeviceLocator; // string
    uint8_t BankLocator; // string
    uint8_t MemoryType; // enum
    uint16_t TypeDetail; // bit field

    // 2.3+
    uint16_t Speed; // varies
    uint8_t Manufacturer; // string
    uint8_t SerialNumber; // string
    uint8_t AssetTag; // string
    uint8_t PartNumber; // string

    // 2.6+
    uint8_t Attributes; // varies

    // 2.7+
    uint32_t ExtendedSize; // varies
    uint16_t ConfiguredMemorySpeed; // varies

    // 2.8+
    uint16_t MinimumVoltage; // varies
    uint16_t MaximumVoltage; // varies
    uint16_t ConfiguredVoltage; // varies

    // 3.2+
    uint8_t MemoryTechnology; // varies
    uint16_t MemoryOperatingMode; // bit field
    uint8_t FirmwareVersion; // string
    uint16_t ModuleManufacturerID; // varies
    uint16_t ModuleProductID; // varies
    uint16_t MemorySubsystemControllerManufacturerID; // vaies
    uint16_t MemorySubsystemControllerProductID; // varies
    uint64_t NonVolatileSize; // varies
    uint64_t VolatileSize; // varies
    uint64_t CacheSize; // varies
    uint64_t LogicalSize; // varies

    // 3.3+
    uint32_t ExtendedSpeed; // varies
    uint32_t ExtendedConfiguredSpeed; // varies

    // 3.7+
    uint16_t Pmic0ManufacturerID; // varies
    uint16_t Pmic0RevisionNumber; // varies
    uint16_t RcdManufacturerID; // varies
    uint16_t RcdRevisionNumber; // varies
} __attribute__((__packed__)) FFSmbiosMemoryDevice;

static_assert(offsetof(FFSmbiosMemoryDevice, RcdRevisionNumber) == 0x62,
    "FFSmbiosMemoryDevice: Wrong struct alignment");

const char* ffDetectPhysicalMemory(FFlist* result)
{
    const FFSmbiosHeaderTable* smbiosTable = ffGetSmbiosHeaderTable();
    if (!smbiosTable)
        return "Failed to get SMBIOS data";

    const FFSmbiosMemoryDevice* data = (const FFSmbiosMemoryDevice*) (*smbiosTable)[FF_SMBIOS_TYPE_MEMORY_DEVICE];
    if (!data)
        return "Memory device is not found in SMBIOS data";

    for (; data->Header.Type < FF_SMBIOS_TYPE_END_OF_TABLE;
        data = (const FFSmbiosMemoryDevice*) ffSmbiosNextEntry(&data->Header))
    {
        if (data->Header.Type != FF_SMBIOS_TYPE_MEMORY_DEVICE || data->Size == 0) continue;

        const char* strings = (const char*) data + data->Header.Length;

        FFPhysicalMemoryResult* device = ffListAdd(result);
        ffStrbufInit(&device->type);
        ffStrbufInit(&device->formFactor);
        ffStrbufInit(&device->locator);
        ffStrbufInit(&device->vendor);
        ffStrbufInit(&device->serial);
        ffStrbufInit(&device->partNumber);
        device->size = 0;
        device->maxSpeed = 0;
        device->runningSpeed = 0;
        device->ecc = false;

        if (data->TotalWidth != 0xFFFF && data->DataWidth != 0xFFFF)
            device->ecc = data->TotalWidth > data->DataWidth;

        if (data->Size != 0xFFFF)
        {
            if (data->Size == 0x7FFF)
                device->size = (data->ExtendedSize & ~(1ULL << 31)) * 1024ULL * 1024ULL;
            else if (data->Size & (1 << 15))
            {
                // in kB
                device->size = (data->Size & ~(1ULL << 15)) * 1024ULL;
            }
            else
            {
                // in MB
                device->size = data->Size * 1024ULL * 1024ULL;
            }
        }

        // https://github.com/fastfetch-cli/fastfetch/issues/1051#issuecomment-2206687345
        const char* lbank = ffSmbiosLocateString(strings, data->BankLocator);
        const char* ldevice = ffSmbiosLocateString(strings, data->DeviceLocator);
        if (lbank && ldevice)
            ffStrbufSetF(&device->locator, "%s/%s", lbank, ldevice);
        else if (lbank)
            ffStrbufSetS(&device->locator, lbank);
        else if (ldevice)
            ffStrbufSetS(&device->locator, ldevice);

        const char* formFactorNames[] = {
            NULL,              // 0x00 (Placeholder for indexing)
            "Other",           // 0x01
            "Unknown",         // 0x02
            "SIMM",            // 0x03
            "SIP",             // 0x04
            "Chip",            // 0x05
            "DIP",             // 0x06
            "ZIP",             // 0x07
            "Proprietary Card",// 0x08
            "DIMM",            // 0x09
            "TSOP",            // 0x0A
            "Row of chips",    // 0x0B
            "RIMM",            // 0x0C
            "SODIMM",          // 0x0D
            "SRIMM",           // 0x0E
            "FBDIMM",          // 0x0F
            "Die",             // 0x10
        };
        if (data->FormFactor > 0 && data->FormFactor < ARRAY_SIZE(formFactorNames))
            ffStrbufSetS(&device->formFactor, formFactorNames[data->FormFactor]);
        else
            ffStrbufSetF(&device->formFactor, "Unknown (%d)", (int) data->FormFactor);

        const char* memoryTypeNames[] = {
            NULL,         // 0x00 (Placeholder for indexing)
            "Other",      // 0x01
            "Unknown",    // 0x02
            "DRAM",       // 0x03
            "EDRAM",      // 0x04
            "VRAM",       // 0x05
            "SRAM",       // 0x06
            "RAM",        // 0x07
            "ROM",        // 0x08
            "FLASH",      // 0x09
            "EEPROM",     // 0x0A
            "FEPROM",     // 0x0B
            "EPROM",      // 0x0C
            "CDRAM",      // 0x0D
            "3DRAM",      // 0x0E
            "SDRAM",      // 0x0F
            "SGRAM",      // 0x10
            "RDRAM",      // 0x11
            "DDR",        // 0x12
            "DDR2",       // 0x13
            "DDR2 FB-DIMM", // 0x14
            "Reserved",   // 0x15
            "Reserved",   // 0x16
            "Reserved",   // 0x17
            "DDR3",       // 0x18
            "FBD2",       // 0x19
            "DDR4",       // 0x1A
            "LPDDR",      // 0x1B
            "LPDDR2",     // 0x1C
            "LPDDR3",     // 0x1D
            "LPDDR4",     // 0x1E
            "Logical non-volatile device", // 0x1F
            "HBM",        // 0x20
            "HBM2",       // 0x21
            "DDR5",       // 0x22
            "LPDDR5",     // 0x23
            "HBM3",       // 0x24
        };
        if (data->MemoryType > 0 && data->MemoryType < ARRAY_SIZE(memoryTypeNames))
            ffStrbufSetStatic(&device->type, memoryTypeNames[data->MemoryType]);
        else
            ffStrbufSetF(&device->type, "Unknown (%d)", (int) data->MemoryType);

        if (data->Header.Length > offsetof(FFSmbiosMemoryDevice, Speed)) // 2.3+
        {
            if (data->Speed)
                device->maxSpeed = data->Speed == 0xFFFF ? data->ExtendedSpeed : data->Speed;

            ffStrbufSetStatic(&device->vendor, ffSmbiosLocateString(strings, data->Manufacturer));
            ffCleanUpSmbiosValue(&device->vendor);
            FFPhysicalMemoryUpdateVendorString(device);

            ffStrbufSetStatic(&device->serial, ffSmbiosLocateString(strings, data->SerialNumber));
            ffCleanUpSmbiosValue(&device->serial);

            ffStrbufSetStatic(&device->partNumber, ffSmbiosLocateString(strings, data->PartNumber));
            ffCleanUpSmbiosValue(&device->partNumber);
        }

        if (data->Header.Length > offsetof(FFSmbiosMemoryDevice, ConfiguredMemorySpeed)) // 2.7+
        {
            if (data->ConfiguredMemorySpeed)
                device->runningSpeed = data->ConfiguredMemorySpeed == 0xFFFF
                    ? data->ExtendedConfiguredSpeed : data->ConfiguredMemorySpeed;
        }
    }

    return NULL;
}
