#pragma once

#include "common/option.h"

typedef enum __attribute__((__packed__)) FFColorsSymbol
{
    FF_COLORS_SYMBOL_BLOCK,
    FF_COLORS_SYMBOL_BACKGROUND,
    FF_COLORS_SYMBOL_CIRCLE,
    FF_COLORS_SYMBOL_DIAMOND,
    FF_COLORS_SYMBOL_SQUARE,
    FF_COLORS_SYMBOL_TRIANGLE,
    FF_COLORS_SYMBOL_STAR,
} FFColorsSymbol;

typedef struct FFBlockConfig
{
    uint8_t width;
    uint8_t range[2];
} FFBlockConfig;

typedef struct FFColorsOptions
{
    FFModuleArgs moduleArgs;

    FFColorsSymbol symbol;
    uint32_t paddingLeft;
    FFBlockConfig block;
} FFColorsOptions;

static_assert(sizeof(FFColorsOptions) <= FF_OPTION_MAX_SIZE, "FFColorsOptions size exceeds maximum allowed size");
