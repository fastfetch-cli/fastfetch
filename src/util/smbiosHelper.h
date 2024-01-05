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

#ifdef __linux__
void ffGetSmbiosValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer);
#elif defined(_WIN32)
// https://github.com/KunYi/DumpSMBIOS
// https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.7.0.pdf

typedef enum __attribute__((__packed__)) FFSmbiosType // : uint8_t
{
    FF_SMBIOS_TYPE_BIOS = 0,
    FF_SMBIOS_TYPE_SYSTEM_INFO = 1,
    FF_SMBIOS_TYPE_BASEBOARD_INFO = 2,
    FF_SMBIOS_TYPE_SYSTEM_ENCLOSURE = 3,
    FF_SMBIOS_TYPE_PROCESSOR_INFO = 4,
    FF_SMBIOS_TYPE_MEMORY_MODULE_INFO = 6,
    FF_SMBIOS_TYPE_CACHE_INFO = 7,
    FF_SMBIOS_TYPE_OEM_STRING = 11,
    FF_SMBIOS_TYPE_MEMORY_DEVICE = 17,
    FF_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS = 19,
    FF_SMBIOS_TYPE_BUILTIN_POINT_DEVICE = 21,
    FF_SMBIOS_TYPE_PORTABLE_BATTERY = 22,
    FF_SMBIOS_TYPE_LAST = 127,
} FFSmbiosType;
static_assert(sizeof(FFSmbiosType) == 1, "FFSmbiosType should be 1 byte");

typedef struct FFSmbiosHeader
{
    FFSmbiosType Type;
    uint8_t Length;
    uint16_t Handle;
    uint8_t Data[];
} FFSmbiosHeader;
static_assert(sizeof(FFSmbiosHeader) == 4, "FFSmbiosHeader should be 4 bytes");

typedef struct FFRawSmbiosData
{
    uint8_t Used20CallingMethod;
    uint8_t SMBIOSMajorVersion;
    uint8_t SMBIOSMinorVersion;
    uint8_t DmiRevision;
    uint32_t Length;
    uint8_t SMBIOSTableData[];
} FFRawSmbiosData;

static inline const char* ffSmbiosLocateString(const char* start, uint8_t index /* start from 1 */)
{
    if (index == 0 || *start == '\0')
        return NULL;
    while (--index)
        start += strlen(start) + 1;
    return start;
}

static inline const FFSmbiosHeader* ffSmbiosSkipLastStr(const FFSmbiosHeader* header)
{
    const char* p = ((const char*) header) + header->Length;
    while (*p)
        p += strlen(p) + 1;

    return (const FFSmbiosHeader*) (p + 1);
}

const FFRawSmbiosData* ffGetSmbiosData();
#endif

#endif
