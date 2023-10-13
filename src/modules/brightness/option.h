#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef struct FFBrightnessOptions
{
    FFModuleBaseInfo moduleInfo;
    FFModuleArgs moduleArgs;

#ifdef __linux__
    FFlist busNos;
#endif
} FFBrightnessOptions;
