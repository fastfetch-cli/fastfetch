#pragma once

#ifndef FF_INCLUDED_detection_bios_bios
#define FF_INCLUDED_detection_bios_bios

#include "fastfetch.h"

typedef struct FFBiosResult
{
    FFstrbuf biosDate;
    FFstrbuf biosRelease;
    FFstrbuf biosVendor;
    FFstrbuf biosVersion;
    FFstrbuf error;
} FFBiosResult;

void ffDetectBios(FFBiosResult* bios);

#endif
