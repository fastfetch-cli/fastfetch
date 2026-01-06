#pragma once

#include "util/option.h"
#include "util/percent.h"

typedef struct FFBtrfsOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
} FFBtrfsOptions;

static_assert(sizeof(FFBtrfsOptions) <= FF_OPTION_MAX_SIZE, "FFBtrfsOptions size exceeds maximum allowed size");
