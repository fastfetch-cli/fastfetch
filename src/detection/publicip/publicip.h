#pragma once

#include "fastfetch.h"
#include "modules/publicip/option.h"

typedef struct FFPublicIpResult
{
    FFstrbuf ip;
    FFstrbuf location;
} FFPublicIpResult;

void ffPreparePublicIp(FFPublicIPOptions* options);
const char* ffDetectPublicIp(FFPublicIPOptions* options, FFPublicIpResult* result);
