#pragma once

#include "fastfetch.h"

typedef struct FFTPMResult
{
    FFstrbuf version;
    FFstrbuf description;
} FFTPMResult;

const char* ffDetectTPM(FFTPMResult* result);
