#pragma once

#include "common/option.h"

typedef struct FFCommandOptions
{
    FFModuleArgs moduleArgs;

    FFstrbuf shell;
    FFstrbuf param;
    FFstrbuf text;
    bool useStdErr;
    bool parallel;
    bool splitLines;
} FFCommandOptions;

static_assert(sizeof(FFCommandOptions) <= FF_OPTION_MAX_SIZE, "FFCommandOptions size exceeds maximum allowed size");
