#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef struct FFOSOptions
{
    const char* moduleName;
    FFModuleArgs moduleArgs;

    #if defined(__linux__) || defined(__FreeBSD__)
    FFstrbuf file;
    #endif
} FFOSOptions;
