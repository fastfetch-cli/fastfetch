#pragma once

#include "common/option.h"

typedef struct FFLogoOptions
{
} FFLogoOptions;

static_assert(sizeof(FFLogoOptions) <= FF_OPTION_MAX_SIZE, "FFLogoOptions size exceeds maximum allowed size");
