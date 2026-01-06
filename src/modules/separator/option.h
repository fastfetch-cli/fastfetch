#pragma once

#include "common/option.h"

typedef struct FFSeparatorOptions
{
    FFstrbuf string;
    FFstrbuf outputColor;
    uint32_t times;
} FFSeparatorOptions;

static_assert(sizeof(FFSeparatorOptions) <= FF_OPTION_MAX_SIZE, "FFSeparatorOptions size exceeds maximum allowed size");
