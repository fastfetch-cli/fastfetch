#pragma once

#ifndef FF_INCLUDED_detection_bios_bios
#define FF_INCLUDED_detection_bios_bios

#include "fastfetch.h"

typedef struct FFBiosResult
{
    FFstrbuf date;
    FFstrbuf biosRelease;
    FFstrbuf vendor;
    FFstrbuf version;
} FFBiosResult;

const char* ffDetectBios(FFBiosResult* bios);

#endif
