#pragma once

#include "common/option.h"

typedef struct FFBiosOptions
{
    FFModuleArgs moduleArgs;
} FFBiosOptions;

static_assert(sizeof(FFBiosOptions) <= FF_OPTION_MAX_SIZE, "FFBiosOptions size exceeds maximum allowed size");
