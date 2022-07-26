#pragma once

#ifndef FF_INCLUDED_detection_title
#define FF_INCLUDED_detection_title

#include "fastfetch.h"

typedef struct FFTitleResult
{
    FFstrbuf userName;
    FFstrbuf hostname;
    FFstrbuf fqdn; //Fully qualified domain name
} FFTitleResult;

const FFTitleResult* ffDetectTitle(const FFinstance* instance);

#endif
