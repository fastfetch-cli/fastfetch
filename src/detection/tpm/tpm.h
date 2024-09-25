#pragma once

#include "fastfetch.h"

typedef struct FFTPMResult
{
    FFstrbuf version;
    FFstrbuf interfaceType;
} FFTPMResult;

const char* ffDetectTPM(FFTPMResult* result);
