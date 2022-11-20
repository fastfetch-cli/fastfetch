#pragma once

#ifndef FF_INCLUDED_detection_host_host
#define FF_INCLUDED_detection_host_host

#include "fastfetch.h"

typedef struct FFHostResult
{
    FFstrbuf productFamily;
    FFstrbuf productName;
    FFstrbuf productVersion;
    FFstrbuf productSku;
    FFstrbuf chassisType;
    FFstrbuf chassisVendor;
    FFstrbuf chassisVersion;
    FFstrbuf sysVendor;
    FFstrbuf error;
} FFHostResult;

const FFHostResult* ffDetectHost();

#endif
