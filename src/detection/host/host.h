#pragma once

#include "fastfetch.h"
#include "modules/host/option.h"

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
#if __x86_64__
bool ffHostDetectMac(FFHostResult* host);
#endif
const char* ffDetectHost(FFHostResult* host);
