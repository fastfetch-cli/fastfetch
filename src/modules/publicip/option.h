#pragma once

#include "common/option.h"

typedef struct FFPublicIPOptions
{
    FFModuleArgs moduleArgs;

    FFstrbuf url;
    uint32_t timeout;
    bool ipv6;
} FFPublicIPOptions;

static_assert(sizeof(FFPublicIPOptions) <= FF_OPTION_MAX_SIZE, "FFPublicIPOptions size exceeds maximum allowed size");
