#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef struct FFSeparatorOptions
{
    FFModuleBaseInfo moduleInfo;

    FFstrbuf string;
    FFstrbuf outputColor;
    uint32_t length;
} FFSeparatorOptions;
