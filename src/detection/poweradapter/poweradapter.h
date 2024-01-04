#pragma once

#include "fastfetch.h"

typedef struct FFPowerAdapterResult
{
    FFstrbuf description;
    FFstrbuf name;
    FFstrbuf modelName;
    FFstrbuf manufacturer;
    int watts;
} FFPowerAdapterResult;

const char* ffDetectPowerAdapter(FFlist* results);

#define FF_POWERADAPTER_UNSET -2
#define FF_POWERADAPTER_NOT_CONNECTED -1
