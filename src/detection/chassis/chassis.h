#pragma once

#include "fastfetch.h"
#include "modules/chassis/option.h"

typedef struct FFChassisResult
{
    FFstrbuf type;
    FFstrbuf serial;
    FFstrbuf vendor;
    FFstrbuf version;
} FFChassisResult;

const char* ffDetectChassis(FFChassisResult* result);
const char* ffChassisTypeToString(uint32_t type);
