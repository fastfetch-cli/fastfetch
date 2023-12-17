#pragma once

#include "fastfetch.h"

typedef enum FFDiskIOPhysicalType
{
    FF_DISKIO_PHYSICAL_TYPE_UNKNOWN,
    FF_DISKIO_PHYSICAL_TYPE_HDD,
    FF_DISKIO_PHYSICAL_TYPE_SSD,
} FFDiskIOPhysicalType;

typedef struct FFDiskIOResult
{
    FFstrbuf name;
    FFstrbuf interconnect;
    FFstrbuf serial;
    FFDiskIOPhysicalType type;
    uint64_t size;
    FFstrbuf devPath;
    uint64_t bytesRead;
    uint64_t readCount;
    uint64_t bytesWritten;
    uint64_t writeCount;
} FFDiskIOResult;

const char* ffDetectDiskIO(FFlist* result, FFDiskIOOptions* options);
