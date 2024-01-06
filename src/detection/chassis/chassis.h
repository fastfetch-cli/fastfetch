#pragma once

#include "fastfetch.h"

typedef struct FFChassisResult
{
    FFstrbuf type;
    FFstrbuf vendor;
    FFstrbuf version;
} FFChassisResult;

const char* ffDetectChassis(FFChassisResult* result);
const char* ffChassisTypeToString(uint32_t type);
