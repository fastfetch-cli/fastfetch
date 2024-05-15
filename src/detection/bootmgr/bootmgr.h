#pragma once

#include "fastfetch.h"

typedef struct FFBootmgrResult
{
    FFstrbuf name;
    FFstrbuf firmware;
    bool secureBoot;
} FFBootmgrResult;

const char* ffDetectBootmgr(FFBootmgrResult* bios);
