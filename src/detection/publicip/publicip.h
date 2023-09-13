#pragma once

#ifndef FF_INCLUDED_detection_publicip_publicip
#define FF_INCLUDED_detection_publicip_publicip

#include "fastfetch.h"

typedef struct FFPublicIpResult
{
    FFstrbuf ip;
    FFstrbuf location;
} FFPublicIpResult;

void ffPreparePublicIp(FFPublicIpOptions* options);
const char* ffDetectPublicIp(FFPublicIpOptions* options, FFPublicIpResult* result);

#endif
