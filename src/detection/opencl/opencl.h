#pragma once

#include "fastfetch.h"

typedef struct FFOpenCLResult
{
    FFstrbuf version;
    FFstrbuf device;
    FFstrbuf vendor;
} FFOpenCLResult;

const char* ffDetectOpenCL(FFOpenCLResult* result);
