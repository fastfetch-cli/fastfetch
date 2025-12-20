#pragma once

#include "fastfetch.h"
#include "modules/zpool/option.h"

typedef struct FFZpoolResult
{
    FFstrbuf name;
    FFstrbuf state;
    uint64_t guid;
    uint64_t used;
    uint64_t total;
    uint64_t allocated;
    double fragmentation;
    bool readOnly;
} FFZpoolResult;

const char* ffDetectZpool(FFlist* result /* list of FFZpoolResult */);
