#pragma once

#include "common/option.h"

typedef struct FFWallpaperOptions
{
    FFModuleArgs moduleArgs;
} FFWallpaperOptions;

static_assert(sizeof(FFWallpaperOptions) <= FF_OPTION_MAX_SIZE, "FFWallpaperOptions size exceeds maximum allowed size");
