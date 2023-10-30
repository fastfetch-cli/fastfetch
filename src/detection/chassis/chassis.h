#pragma once

#ifndef FF_INCLUDED_detection_chassis_chassis
#define FF_INCLUDED_detection_chassis_chassis

#include "fastfetch.h"

typedef struct FFChassisResult
{
    FFstrbuf type;
    FFstrbuf vendor;
    FFstrbuf version;
} FFChassisResult;

const char* ffDetectChassis(FFChassisResult* result, FFChassisOptions* options);
const char* ffChassisTypeToString(uint32_t type);

#endif
