#pragma once

#include "util/option.h"

typedef struct FFUsersOptions
{
    FFModuleArgs moduleArgs;

    bool compact;
    bool myselfOnly;
} FFUsersOptions;

static_assert(sizeof(FFUsersOptions) <= FF_OPTION_MAX_SIZE, "FFUsersOptions size exceeds maximum allowed size");
