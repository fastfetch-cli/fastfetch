#pragma once

#ifndef FASTFETCH_INCLUDED_SMBIOSVALUEHELPER
#define FASTFETCH_INCLUDED_SMBIOSVALUEHELPER

#include "util/FFstrbuf.h"

bool ffIsSmbiosValueSet(FFstrbuf* value);
static inline void ffCleanUpSmbiosValue(FFstrbuf* value)
{
    if (!ffIsSmbiosValueSet(value))
        ffStrbufClear(value);
}

// https://github.com/KunYi/DumpSMBIOS
// https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.7.0.pdf

typedef enum __attribute__((__packed__)) FFSmbiosType // : uint8_t
{
    FF_SMBIOS_TYPE_BIOS = 0,
    FF_SMBIOS_TYPE_SYSTEM_INFO = 1,
    FF_SMBIOS_TYPE_BASEBOARD_INFO = 2,
    FF_SMBIOS_TYPE_SYSTEM_ENCLOSURE = 3,
    FF_SMBIOS_TYPE_PROCESSOR_INFO = 4,
    FF_SMBIOS_TYPE_MEMORY_CONTROLLER_INFO = 5, // obsolete
    FF_SMBIOS_TYPE_MEMORY_MODULE_INFO = 6, // obsolete
    FF_SMBIOS_TYPE_CACHE_INFO = 7,
    FF_SMBIOS_TYPE_PORT_CONNECTOR_INFO = 8,
    FF_SMBIOS_TYPE_SYSTEM_SLOTS = 9,
    FF_SMBIOS_TYPE_ON_BOARD_DEVICES_INFO = 10, // obsolete
    FF_SMBIOS_TYPE_OEM_STRING = 11,
    FF_SMBIOS_TYPE_SYSTEM_CONFIGURATION_OPTIONS = 12,
    FF_SMBIOS_TYPE_BIOS_LANGUAGE_INFO = 13,
    FF_SMBIOS_TYPE_GROUP_ASSOCIATIONS = 14,
    FF_SMBIOS_TYPE_SYSTEM_EVENT_LOG = 15,
    FF_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY = 16,
    FF_SMBIOS_TYPE_MEMORY_DEVICE = 17,
    FF_SMBIOS_TYPE_32BIT_MEMORY_ERROR_INFO = 18,
    FF_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS = 19,
    FF_SMBIOS_TYPE_MEMORY_DEVICE_MAPPED_ADDRESS = 20,
    FF_SMBIOS_TYPE_BUILTIN_POINTING_DEVICE = 21,
    FF_SMBIOS_TYPE_PORTABLE_BATTERY = 22,
    FF_SMBIOS_TYPE_SYSTEM_RESET = 23,
    FF_SMBIOS_TYPE_HARDWARE_SECURITY = 24,
    FF_SMBIOS_TYPE_SYSTEM_POWER_CONTROLS = 25,
    FF_SMBIOS_TYPE_VOLTAGE_PROBE = 26,
    FF_SMBIOS_TYPE_COOLING_DEVICE = 27,
    FF_SMBIOS_TYPE_TEMPERATURE_PROBE = 28,
    FF_SMBIOS_TYPE_ELECTRICAL_CURRENT_PROBE = 29,
    FF_SMBIOS_TYPE_OUT_OF_BAND_REMOTE_ACCESS = 30,
    FF_SMBIOS_TYPE_BOOT_INTEGRITY_SERVICES_ENTRY_POINT = 31, // reserved
    FF_SMBIOS_TYPE_SYSTEM_BOOT_INFO = 32,
    FF_SMBIOS_TYPE_64BIT_MEMORY_ERROR_INFO = 33,
    FF_SMBIOS_TYPE_MANAGEMENT_DEVICE = 34,
    FF_SMBIOS_TYPE_MANAGEMENT_DEVICE_COMPONENT = 35,
    FF_SMBIOS_TYPE_MANAGEMENT_DEVICE_THRESHOLD_DATA = 36,
    FF_SMBIOS_TYPE_MEMORY_CHANNEL = 37,
    FF_SMBIOS_TYPE_IPMI_DEVICE_INFO = 38,
    FF_SMBIOS_TYPE_SYSTEM_POWER_SUPPLY = 39,
    FF_SMBIOS_TYPE_ADDITIONAL_INFO = 40,
    FF_SMBIOS_TYPE_ONBOARD_DEVICE_EXTENDED_INFO = 41,
    FF_SMBIOS_TYPE_MANAGEMENT_CONTROLLER_HOST_INTERFACE = 42,
    FF_SMBIOS_TYPE_TPM_DEVICE = 43,
    FF_SMBIOS_TYPE_PROCESSOR_ADDITIONAL_INFO = 44,
    FF_SMBIOS_TYPE_FIRMWARE_INVENTORY_INFO = 45,
    FF_SMBIOS_TYPE_STRING_PROPERTY = 46,
    FF_SMBIOS_TYPE_INACTIVE = 126,
    FF_SMBIOS_TYPE_END_OF_TABLE = 127,
    // system- and OEM-specific information 128~256
} FFSmbiosType;
static_assert(sizeof(FFSmbiosType) == 1, "FFSmbiosType should be 1 byte");

typedef struct FFSmbiosHeader
{
    FFSmbiosType Type;
    uint8_t Length;
    uint16_t Handle;
} __attribute__((__packed__)) FFSmbiosHeader;
static_assert(sizeof(FFSmbiosHeader) == 4, "FFSmbiosHeader should be 4 bytes");

static inline const char* ffSmbiosLocateString(const char* start, uint8_t index /* start from 1 */)
{
    if (index == 0 || *start == '\0')
        return NULL;
    while (--index)
        start += strlen(start) + 1;
    return start;
}

typedef const FFSmbiosHeader* FFSmbiosHeaderTable[FF_SMBIOS_TYPE_END_OF_TABLE];

const FFSmbiosHeader* ffSmbiosNextEntry(const FFSmbiosHeader* header);
const FFSmbiosHeaderTable* ffGetSmbiosHeaderTable();

#ifdef __linux__
bool ffGetSmbiosValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer);
#endif

#endif
