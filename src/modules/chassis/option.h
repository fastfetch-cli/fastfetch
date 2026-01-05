#pragma once

#include "util/option.h"

typedef struct FFChassisOptions
{
    FFModuleArgs moduleArgs;
} FFChassisOptions;

static_assert(sizeof(FFChassisOptions) <= FF_OPTION_MAX_SIZE, "FFChassisOptions size exceeds maximum allowed size");
