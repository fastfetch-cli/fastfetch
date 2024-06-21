#include "cpucache.h"
#include "util/smbiosHelper.h"
#include "util/stringUtils.h"

typedef struct FFSmbiosCacheInfo
{
    FFSmbiosHeader Header;

    uint8_t SocketDesignation; // string
    uint16_t CacheConfiguration; // varies
    uint16_t MaximumCacheSize; // varies
    uint16_t InstalledSize; // varies
    uint16_t SupportedSramType; // bit field
    uint16_t CurrentSramType; // bit field

    // 2.1+
    uint8_t CacheSpeed; // varies
    uint8_t ErrorCorrectionType; // enum
    uint8_t SystemCacheType; // enum
    uint8_t Associativity; // enum

    // 3.1+
    uint32_t MaximumCacheSize2; // bit field
    uint32_t InstalledCacheSize2; // bit field
} __attribute__((__packed__)) FFSmbiosCacheInfo;

static_assert(offsetof(FFSmbiosCacheInfo, InstalledCacheSize2) == 0x17,
    "FFSmbiosCacheInfo: Wrong struct alignment");

const char* ffDetectCPUCache(FFCPUCacheResult* result)
{
    const FFSmbiosHeaderTable* smbiosTable = ffGetSmbiosHeaderTable();
    if (!smbiosTable)
        return "Failed to get SMBIOS data";

    const FFSmbiosCacheInfo* data = (const FFSmbiosCacheInfo*) (*smbiosTable)[FF_SMBIOS_TYPE_CACHE_INFO];
    if (!data)
        return "Cache information is not found in SMBIOS data";

    for (; data->Header.Type == FF_SMBIOS_TYPE_CACHE_INFO;
           data = (const FFSmbiosCacheInfo*) ffSmbiosNextEntry(&data->Header))
    {
        bool enabled = !!(data->CacheConfiguration & (1 << 7));
        if (!enabled)
            continue;

        uint32_t size = data->InstalledSize;
        if (size == 0)
            continue;

        if (data->InstalledSize != 0xFFFF)
        {
            size *= (size >> 15 ? 64 : 1) * 1024u;
        }
        else if (data->Header.Length > offsetof(FFSmbiosCacheInfo, InstalledCacheSize2))
        {
            size = data->InstalledCacheSize2;
            size *= (size >> 31 ? 64 : 1) * 1024u;
        }

        uint32_t level = (data->CacheConfiguration & 0b111u) + 1;

        FFCPUCacheType type;
        switch (data->SystemCacheType)
        {
            case 3: type = FF_CPU_CACHE_TYPE_INSTRUCTION; break;
            case 4: type = FF_CPU_CACHE_TYPE_DATA; break;
            default: type = FF_CPU_CACHE_TYPE_UNIFIED; break;
        }

        ffCPUCacheAddItem(result, level, size, 0, type);
    }

    return NULL;
}
