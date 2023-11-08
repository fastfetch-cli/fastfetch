#pragma once

#include "util/FFstrbuf.h"

const char* ffProcessAppendOutput(FFstrbuf* buffer, char* const argv[], bool useStdErr);

static inline const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[])
{
    const char* error = ffProcessAppendOutput(buffer, argv, false);
    if (!error)
    {
        ffStrbufTrimRight(buffer, '\n');
        ffStrbufTrimRight(buffer, ' ');
    }
    return error;
}

static inline const char* ffProcessAppendStdErr(FFstrbuf* buffer, char* const argv[])
{
    const char* error = ffProcessAppendOutput(buffer, argv, true);
    if (!error)
    {
        ffStrbufTrimRight(buffer, '\n');
        ffStrbufTrimRight(buffer, ' ');
    }
    return error;
}
