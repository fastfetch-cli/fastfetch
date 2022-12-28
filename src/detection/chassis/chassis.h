#pragma once

#ifndef FF_INCLUDED_detection_chassis_chassis
#define FF_INCLUDED_detection_chassis_chassis

#include "fastfetch.h"

typedef struct FFChassisResult
{
    FFstrbuf chassisType;
    FFstrbuf chassisVendor;
    FFstrbuf chassisVersion;
    FFstrbuf error;
} FFChassisResult;

void ffDetectChassis(FFChassisResult* result);

#endif
