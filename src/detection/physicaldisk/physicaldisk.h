#include "fastfetch.h"
#include "modules/physicaldisk/option.h"

#define FF_PHYSICALDISK_TEMP_UNSET (-DBL_MAX)

typedef struct FFPhysicalDiskResult {
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
