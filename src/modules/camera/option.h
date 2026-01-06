#pragma once

#include "util/option.h"

typedef struct FFCameraOptions
{
    FFModuleArgs moduleArgs;
} FFCameraOptions;

static_assert(sizeof(FFCameraOptions) <= FF_OPTION_MAX_SIZE, "FFCameraOptions size exceeds maximum allowed size");
