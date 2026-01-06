#pragma once

#include "common/option.h"

typedef struct FFFontOptions
{
    FFModuleArgs moduleArgs;
} FFFontOptions;

static_assert(sizeof(FFFontOptions) <= FF_OPTION_MAX_SIZE, "FFFontOptions size exceeds maximum allowed size");
