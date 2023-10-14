#pragma once

#include "fastfetch.h"

typedef struct FFNetIOResult
{
    FFstrbuf name;
    bool defaultRoute;
    uint64_t txBytes;
    uint64_t rxBytes;
    uint64_t txPackets;
    uint64_t rxPackets;
    uint64_t rxErrors;
    uint64_t txErrors;
    uint64_t rxDrops;
    uint64_t txDrops;
} FFNetIOResult;

const char* ffDetectNetIO(FFlist* result, FFNetIOOptions* options);
