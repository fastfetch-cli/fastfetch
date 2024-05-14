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

    for (; data->Header.Type == FF_SMBIOS_TYPE_MEMORY_DEVICE;
           data = (const FFSmbiosMemoryDevice*) ffSmbiosNextEntry(&data->Header))
    {
        if (data->Size == 0) continue;

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
                device->size = (data->ExtendedSize & ~(1 << 31)) * 1024 * 1024;
            else if (data->Size & (1 << 15))
            {
                // in kB
                device->size = (data->Size & ~(1u << 15)) * 1024ULL;
            }
            else
            {
                // in MB
                device->size = data->Size * 1024ULL * 1024ULL;
            }
        }

        ffStrbufSetF(&device->locator, "%s/%s", ffSmbiosLocateString(strings, data->BankLocator), ffSmbiosLocateString(strings, data->DeviceLocator));

        switch (data->FormFactor)
        {
            case 0x01: ffStrbufSetStatic(&device->formFactor, "Other"); break;
            case 0x02: ffStrbufSetStatic(&device->formFactor, "Unknown"); break;
            case 0x03: ffStrbufSetStatic(&device->formFactor, "SIMM"); break;
            case 0x04: ffStrbufSetStatic(&device->formFactor, "SIP"); break;
            case 0x05: ffStrbufSetStatic(&device->formFactor, "Chip"); break;
            case 0x06: ffStrbufSetStatic(&device->formFactor, "DIP"); break;
            case 0x07: ffStrbufSetStatic(&device->formFactor, "ZIP"); break;
            case 0x08: ffStrbufSetStatic(&device->formFactor, "Proprietary Card"); break;
            case 0x09: ffStrbufSetStatic(&device->formFactor, "DIMM"); break;
            case 0x0A: ffStrbufSetStatic(&device->formFactor, "TSOP"); break;
            case 0x0B: ffStrbufSetStatic(&device->formFactor, "Row of chips"); break;
            case 0x0C: ffStrbufSetStatic(&device->formFactor, "RIMM"); break;
            case 0x0D: ffStrbufSetStatic(&device->formFactor, "SODIMM"); break;
            case 0x0E: ffStrbufSetStatic(&device->formFactor, "SRIMM"); break;
            case 0x0F: ffStrbufSetStatic(&device->formFactor, "FBDIMM"); break;
            case 0x10: ffStrbufSetStatic(&device->formFactor, "Die"); break;
            default: ffStrbufSetF(&device->formFactor, "Unknown (%d)", (int) data->FormFactor); break;
        }

        switch (data->MemoryType)
        {
            case 0x01: ffStrbufSetStatic(&device->type, "Other"); break;
            case 0x02: ffStrbufSetStatic(&device->type, "Unknown"); break;
            case 0x03: ffStrbufSetStatic(&device->type, "DRAM"); break;
            case 0x04: ffStrbufSetStatic(&device->type, "EDRAM"); break;
            case 0x05: ffStrbufSetStatic(&device->type, "VRAM"); break;
            case 0x06: ffStrbufSetStatic(&device->type, "SRAM"); break;
            case 0x07: ffStrbufSetStatic(&device->type, "RAM"); break;
            case 0x08: ffStrbufSetStatic(&device->type, "ROM"); break;
            case 0x09: ffStrbufSetStatic(&device->type, "FLASH"); break;
            case 0x0A: ffStrbufSetStatic(&device->type, "EEPROM"); break;
            case 0x0B: ffStrbufSetStatic(&device->type, "FEPROM"); break;
            case 0x0C: ffStrbufSetStatic(&device->type, "EPROM"); break;
            case 0x0D: ffStrbufSetStatic(&device->type, "CDRAM"); break;
            case 0x0E: ffStrbufSetStatic(&device->type, "3DRAM"); break;
            case 0x0F: ffStrbufSetStatic(&device->type, "SDRAM"); break;
            case 0x10: ffStrbufSetStatic(&device->type, "SGRAM"); break;
            case 0x11: ffStrbufSetStatic(&device->type, "RDRAM"); break;
            case 0x12: ffStrbufSetStatic(&device->type, "DDR"); break;
            case 0x13: ffStrbufSetStatic(&device->type, "DDR2"); break;
            case 0x14: ffStrbufSetStatic(&device->type, "DDR2 FB-DIMM"); break;
            case 0x15:
            case 0x16:
            case 0x17: ffStrbufSetStatic(&device->type, "Reserved"); break;
            case 0x18: ffStrbufSetStatic(&device->type, "DDR3"); break;
            case 0x19: ffStrbufSetStatic(&device->type, "FBD2"); break;
            case 0x1A: ffStrbufSetStatic(&device->type, "DDR4"); break;
            case 0x1B: ffStrbufSetStatic(&device->type, "LPDDR"); break;
            case 0x1C: ffStrbufSetStatic(&device->type, "LPDDR2"); break;
            case 0x1D: ffStrbufSetStatic(&device->type, "LPDDR3"); break;
            case 0x1E: ffStrbufSetStatic(&device->type, "LPDDR4"); break;
            case 0x1F: ffStrbufSetStatic(&device->type, "Logical non-volatile device"); break;
            case 0x20: ffStrbufSetStatic(&device->type, "HBM"); break;
            case 0x21: ffStrbufSetStatic(&device->type, "HBM2"); break;
            case 0x22: ffStrbufSetStatic(&device->type, "DDR5"); break;
            case 0x23: ffStrbufSetStatic(&device->type, "LPDDR5"); break;
            case 0x24: ffStrbufSetStatic(&device->type, "HBM3"); break;
            default: ffStrbufSetF(&device->type, "Unknown (%d)", (int) data->MemoryType); break;
        }

        if (data->Header.Length > offsetof(FFSmbiosMemoryDevice, Speed)) // 2.3+
        {
            if (data->Speed)
                device->maxSpeed = data->Speed == 0xFFFF ? data->ExtendedSpeed : data->Speed;

            ffStrbufSetStatic(&device->vendor, ffSmbiosLocateString(strings, data->Manufacturer));
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
