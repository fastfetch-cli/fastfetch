#pragma once

#include "util/option.h"

typedef struct FFBreakOptions
{
} FFBreakOptions;

static_assert(sizeof(FFBreakOptions) <= FF_OPTION_MAX_SIZE, "FFBreakOptions size exceeds maximum allowed size");
