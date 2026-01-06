#pragma once

#include "common/option.h"

typedef struct FFEditorOptions
{
    FFModuleArgs moduleArgs;
} FFEditorOptions;

static_assert(sizeof(FFEditorOptions) <= FF_OPTION_MAX_SIZE, "FFEditorOptions size exceeds maximum allowed size");
