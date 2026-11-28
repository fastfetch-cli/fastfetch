#include "fastfetch.h"
#include "modules/physicaldisk/option.h"

#define FF_PHYSICALDISK_TEMP_UNSET (-DBL_MAX)
#define FF_PHYSICALDISK_PERCENT_UNSET UINT8_MAX

// Bits 0-5 match NVMe Critical Warning
typedef enum FFPhysicalDiskWarning: uint8_t {
    FF_PHYSICALDISK_WARNING_NONE = 0,
    FF_PHYSICALDISK_WARNING_SPARE = 1 << 0,
    FF_PHYSICALDISK_WARNING_TEMPERATURE = 1 << 1,
    FF_PHYSICALDISK_WARNING_RELIABILITY = 1 << 2,
    FF_PHYSICALDISK_WARNING_READONLY = 1 << 3,
    FF_PHYSICALDISK_WARNING_BACKUP = 1 << 4,
    FF_PHYSICALDISK_WARNING_PMR_READONLY = 1 << 5,
    FF_PHYSICALDISK_WARNING_FAILING = 1 << 6,
    FF_PHYSICALDISK_WARNING_UNSET = UINT8_MAX,
} FFPhysicalDiskWarning;
static_assert(sizeof(FFPhysicalDiskWarning) == sizeof(uint8_t), "");

typedef struct FFPhysicalDiskResult {
    FFstrbuf name;
    FFstrbuf interconnect;
    FFstrbuf serial;
    FFstrbuf devPath;
    FFstrbuf revision;
    FFPhysicalDiskType type;
    uint64_t size;
    double temperature;
    uint8_t percentUsed;
    uint8_t availableSpare;
    FFPhysicalDiskWarning criticalWarning;
} FFPhysicalDiskResult;

const char* ffDetectPhysicalDisk(FFlist* result, FFPhysicalDiskOptions* options);
