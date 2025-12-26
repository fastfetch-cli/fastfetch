#pragma once

#include "common/option.h"

typedef struct FFSecurityOptions
{
    FFModuleArgs moduleArgs;
} FFSecurityOptions;

static_assert(sizeof(FFSecurityOptions) <= FF_OPTION_MAX_SIZE, "FFSecurityOptions size exceeds maximum allowed size");
