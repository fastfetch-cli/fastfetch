#pragma once

#include "common/option.h"

typedef struct FFTitleOptions
{
    FFModuleArgs moduleArgs;

    FFstrbuf colorUser;
    FFstrbuf colorAt;
    FFstrbuf colorHost;
    bool fqdn;
} FFTitleOptions;

static_assert(sizeof(FFTitleOptions) <= FF_OPTION_MAX_SIZE, "FFTitleOptions size exceeds maximum allowed size");
