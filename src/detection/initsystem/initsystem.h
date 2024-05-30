#pragma once

#include "fastfetch.h"

typedef struct FFInitSystemResult
{
    FFstrbuf name;
    FFstrbuf exe;
    FFstrbuf version;
    uint32_t pid;
} FFInitSystemResult;

const char* ffDetectInitSystem(FFInitSystemResult* result);
