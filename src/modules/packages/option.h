#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef struct FFPackagesOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

#ifdef _WIN32
    bool winget;
#endif
} FFPackagesOptions;
