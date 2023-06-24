#pragma once

#ifndef FASTFETCH_INCLUDED_SMBIOSVALUEHELPER
#define FASTFETCH_INCLUDED_SMBIOSVALUEHELPER

#include "util/FFstrbuf.h"

bool ffIsSmbiosValueSet(FFstrbuf* value);
static inline void ffCleanUpSmbiosValue(FFstrbuf* value)
{
    if (!ffIsSmbiosValueSet(value))
        ffStrbufClear(value);
}

#endif