#pragma once

#include "fastfetch.h"

typedef struct FFInitSystemResult
{
    uint32_t pid;
    FFstrbuf name;
    FFstrbuf exe;
} FFInitSystemResult;

const char* ffDetectInitSystem(FFInitSystemResult* result);
