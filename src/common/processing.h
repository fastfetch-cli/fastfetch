#pragma once

#ifndef FF_INCLUDED_common_processing
#define FF_INCLUDED_common_processing

#include "util/FFstrbuf.h"

const char* ffProcessAppendOutput(FFstrbuf* buffer, char* const argv[], bool useStdErr);

static inline const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[])
{
    return ffProcessAppendOutput(buffer, argv, false);
}

static inline const char* ffProcessAppendStdErr(FFstrbuf* buffer, char* const argv[])
{
    return ffProcessAppendOutput(buffer, argv, true);
}

#endif
