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

const char* ffDetectHost(FFHostResult* host);
