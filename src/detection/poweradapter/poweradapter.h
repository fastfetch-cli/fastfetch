#pragma once

#include "fastfetch.h"
#include "modules/poweradapter/option.h"

typedef struct FFPowerAdapterResult
{
    FFstrbuf description;
    FFstrbuf name;
    FFstrbuf modelName;
    FFstrbuf manufacturer;
    FFstrbuf serial;
    int watts;
} FFPowerAdapterResult;

const char* ffDetectPowerAdapter(FFlist* results);
