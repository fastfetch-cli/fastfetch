#pragma once

#ifndef FF_INCLUDED_detection_poweradapter_poweradapter
#define FF_INCLUDED_detection_poweradapter_poweradapter

#include "fastfetch.h"

typedef struct FFPowerAdapterResult
{
    FFstrbuf description;
    FFstrbuf name;
    FFstrbuf modelName;
    FFstrbuf manufacturer;
    int watts;
} FFPowerAdapterResult;

const char* ffDetectPowerAdapterImpl(FFlist* results);

#define FF_POWERADAPTER_UNSET -2
#define FF_POWERADAPTER_NOT_CONNECTED -1

#endif
