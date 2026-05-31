#pragma once

#include "common/option.h"

typedef struct FFDecoderOptions {
    FFModuleArgs moduleArgs;
} FFDecoderOptions;

static_assert(sizeof(FFDecoderOptions) <= FF_OPTION_MAX_SIZE, "FFDecoderOptions size exceeds maximum allowed size");
