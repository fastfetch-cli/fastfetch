#pragma once

// This file will be included in "fastfetch.h", do NOT put unnecessary things here

#include "common/option.h"

typedef enum FFColorssymbol
{
    FF_COLORS_SYMBOL_BLOCK,
    FF_COLORS_SYMBOL_CIRCLE,
    FF_COLORS_SYMBOL_DIAMOND,
    FF_COLORS_SYMBOL_SQUARE,
    FF_COLORS_SYMBOL_TRIANGLE,
    FF_COLORS_SYMBOL_STAR,
} FFColorssymbol;

typedef struct FFColorsOptions
{
    const char* moduleName;
    FFColorssymbol symbol;
} FFColorsOptions;
