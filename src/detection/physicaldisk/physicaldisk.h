#include "fastfetch.h"
#include "modules/physicaldisk/option.h"

#define FF_PHYSICALDISK_TEMP_UNSET (-DBL_MAX)

typedef enum __attribute__((__packed__)) FFPhysicalDiskType
{
    FF_PHYSICALDISK_TYPE_NONE = 0,

    // If neither is set, it's unknown
    FF_PHYSICALDISK_TYPE_HDD = 1 << 0,
    FF_PHYSICALDISK_TYPE_SSD = 1 << 1,

    FF_PHYSICALDISK_TYPE_FIXED = 1 << 2,
    FF_PHYSICALDISK_TYPE_REMOVABLE = 1 << 3,

    FF_PHYSICALDISK_TYPE_READWRITE = 1 << 4,
    FF_PHYSICALDISK_TYPE_READONLY = 1 << 5,

    FF_PHYSICALDISK_TYPE_FORCE_UNSIGNED = UINT8_MAX,
} FFPhysicalDiskType;
static_assert(sizeof(FFPhysicalDiskType) == sizeof(uint8_t), "");

typedef struct FFPhysicalDiskResult
{
    FFstrbuf name;
    FFstrbuf interconnect;
    FFstrbuf serial;
    FFstrbuf devPath;
    FFstrbuf revision;
    FFPhysicalDiskType type;
    uint64_t size;
    double temperature;
} FFPhysicalDiskResult;

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options);
