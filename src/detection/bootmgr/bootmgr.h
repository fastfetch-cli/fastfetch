#pragma once

#include "fastfetch.h"

typedef struct FFBootmgrResult
{
    FFstrbuf name;
    FFstrbuf firmware;
    uint16_t order;
    bool secureBoot;
} FFBootmgrResult;

const char* ffDetectBootmgr(FFBootmgrResult* bios);
