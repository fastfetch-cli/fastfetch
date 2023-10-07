#pragma once

#include "fastfetch.h"

typedef struct FFDiskIOResult
{
    FFstrbuf name;
    FFstrbuf type;
    FFstrbuf devPath;
    uint64_t bytesRead;
    uint64_t readCount;
    uint64_t bytesWritten;
    uint64_t writeCount;
} FFDiskIOResult;

const char* ffDetectDiskIO(FFlist* result, FFDiskIOOptions* options);
