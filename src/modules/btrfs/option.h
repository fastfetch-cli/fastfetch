#pragma once

#include "common/option.h"
#include "common/percent.h"

typedef struct FFBtrfsOptions
{
    FFModuleArgs moduleArgs;

    FFPercentageModuleConfig percent;
} FFBtrfsOptions;

static_assert(sizeof(FFBtrfsOptions) <= FF_OPTION_MAX_SIZE, "FFBtrfsOptions size exceeds maximum allowed size");
