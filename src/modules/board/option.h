#pragma once

#include "common/option.h"

typedef struct FFBoardOptions
{
    FFModuleArgs moduleArgs;
} FFBoardOptions;

static_assert(sizeof(FFBoardOptions) <= FF_OPTION_MAX_SIZE, "FFBoardOptions size exceeds maximum allowed size");
