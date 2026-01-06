#pragma once

#include "common/option.h"

typedef struct FFVulkanOptions
{
    FFModuleArgs moduleArgs;
} FFVulkanOptions;

static_assert(sizeof(FFVulkanOptions) <= FF_OPTION_MAX_SIZE, "FFVulkanOptions size exceeds maximum allowed size");
