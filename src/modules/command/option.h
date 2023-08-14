#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef struct FFCommandOptions
{
    const char* moduleName;
    FFModuleArgs moduleArgs;

    FFstrbuf shell;
    FFstrbuf text;
} FFCommandOptions;
