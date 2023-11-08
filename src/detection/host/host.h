#pragma once

#include "fastfetch.h"

typedef struct FFHostResult
{
    FFstrbuf productFamily;
    FFstrbuf productName;
    FFstrbuf productVersion;
    FFstrbuf productSku;
    FFstrbuf sysVendor;
} FFHostResult;

const char* ffDetectHost(FFHostResult* result);
