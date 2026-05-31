#pragma once

#include "common/option.h"

typedef struct FFCodecOptions {
    FFModuleArgs moduleArgs;
} FFCodecOptions;

static_assert(sizeof(FFCodecOptions) <= FF_OPTION_MAX_SIZE, "FFCodecOptions size exceeds maximum allowed size");
