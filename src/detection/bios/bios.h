#pragma once

#include "fastfetch.h"
#include "modules/bios/option.h"

typedef struct FFBiosResult
{
    FFstrbuf date;
    FFstrbuf release;
    FFstrbuf vendor;
    FFstrbuf version;
    FFstrbuf type;
} FFBiosResult;

const char* ffDetectBios(FFBiosResult* bios);
