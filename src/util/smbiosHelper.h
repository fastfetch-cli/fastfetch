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

#ifdef __linux__
void ffGetSmbiosValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer);
#endif

#endif
