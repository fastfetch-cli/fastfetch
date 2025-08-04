#pragma once

#include "fastfetch.h"
#include "modules/tpm/option.h"

typedef struct FFTPMResult
{
    FFstrbuf version;
    FFstrbuf description;
} FFTPMResult;

const char* ffDetectTPM(FFTPMResult* result);
