#pragma once

#include "common/option.h"
#include "util/FFlist.h"

typedef struct FFKeyboardOptions
{
    FFModuleArgs moduleArgs;

    FFlist ignores; // List of FFstrbuf
} FFKeyboardOptions;

static_assert(sizeof(FFKeyboardOptions) <= FF_OPTION_MAX_SIZE, "FFKeyboardOptions size exceeds maximum allowed size");
