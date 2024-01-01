#include "fastfetch.h"

#define FF_PHYSICALDISK_TEMP_UNSET (0/0.0)

typedef enum FFPhysicalDiskType
{
    FF_PHYSICALDISK_TYPE_NONE = 0,

    // If neither is set, it's unknown
    FF_PHYSICALDISK_TYPE_HDD = 1 << 0,
    FF_PHYSICALDISK_TYPE_SSD = 1 << 1,

    FF_PHYSICALDISK_TYPE_FIXED = 1 << 2,
    FF_PHYSICALDISK_TYPE_REMOVABLE = 1 << 3,

    FF_PHYSICALDISK_TYPE_READWRITE = 1 << 4,
    FF_PHYSICALDISK_TYPE_READONLY = 1 << 5,
} FFPhysicalDiskType;

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
