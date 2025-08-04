#pragma once

#include "common/option.h"

typedef struct FFDateTimeOptions
{
    FFModuleArgs moduleArgs;
} FFDateTimeOptions;

static_assert(sizeof(FFDateTimeOptions) <= FF_OPTION_MAX_SIZE, "FFDateTimeOptions size exceeds maximum allowed size");
