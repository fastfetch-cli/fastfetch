#pragma once

#include "fastfetch.h"

typedef struct FFChassisResult
{
    FFstrbuf type;
    FFstrbuf vendor;
    FFstrbuf version;
} FFChassisResult;

const char* ffDetectChassis(FFChassisResult* result, FFChassisOptions* options);
const char* ffChassisTypeToString(uint32_t type);
