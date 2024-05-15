#pragma once

#include "fastfetch.h"

typedef struct FFBiosResult
{
    FFstrbuf date;
    FFstrbuf release;
    FFstrbuf vendor;
    FFstrbuf version;
    FFstrbuf type;
    FFstrbuf bootmgr;
    bool secureBoot;
} FFBiosResult;

const char* ffDetectBios(FFBiosResult* bios);
