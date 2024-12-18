#pragma once

#include "fastfetch.h"

typedef struct FFHostResult
{
    FFstrbuf family;
    FFstrbuf name;
    FFstrbuf version;
    FFstrbuf sku;
    FFstrbuf serial;
    FFstrbuf uuid;
    FFstrbuf vendor;
} FFHostResult;

const char* ffHostGetMacProductNameWithHwModel(const FFstrbuf* hwModel);
#ifdef __x86_64__
bool ffHostDetectMac(FFHostResult* host);
#endif
const char* ffDetectHost(FFHostResult* host);
