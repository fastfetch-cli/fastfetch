#pragma once

#include "fastfetch.h"

typedef struct FFPublicIpResult
{
    FFstrbuf ip;
    FFstrbuf location;
} FFPublicIpResult;

void ffPreparePublicIp(FFPublicIpOptions* options);
const char* ffDetectPublicIp(FFPublicIpOptions* options, FFPublicIpResult* result);
