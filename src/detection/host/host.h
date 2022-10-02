#pragma once

#ifndef FF_INCLUDED_detection_host_host
#define FF_INCLUDED_detection_host_host

#include "fastfetch.h"

#define FF_HOST_PRODUCT_NAME_WSL "Windows Subsystem for Linux"
#define FF_HOST_PRODUCT_NAME_MSYS "Windows on MSYS"

typedef struct FFHostResult
{
    FFstrbuf productFamily;
    FFstrbuf productName;
    FFstrbuf productVersion;
    FFstrbuf productSku;
    FFstrbuf biosDate;
    FFstrbuf biosRelease;
    FFstrbuf biosVendor;
    FFstrbuf biosVersion;
    FFstrbuf boardName;
    FFstrbuf boardVendor;
    FFstrbuf boardVersion;
    FFstrbuf chassisType;
    FFstrbuf chassisVendor;
    FFstrbuf chassisVersion;
    FFstrbuf sysVendor;
} FFHostResult;

const FFHostResult* ffDetectHost();

#endif
