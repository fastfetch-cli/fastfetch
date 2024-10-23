#pragma once

#include "util/FFstrbuf.h"
#include <wchar.h>

void ffStrbufSetNWS(FFstrbuf* result, uint32_t length, const wchar_t* source);

static inline void ffStrbufSetWS(FFstrbuf* result, const wchar_t* source)
{
    if (!source) return ffStrbufClear(result);
    return ffStrbufSetNWS(result, (uint32_t)wcslen(source), source);
}

void ffStrbufInitNWS(FFstrbuf* result, uint32_t length, const wchar_t* source);

static inline void ffStrbufInitWS(FFstrbuf* result, const wchar_t* source)
{
    if (!source) return ffStrbufInit(result);
    return ffStrbufInitNWS(result, (uint32_t)wcslen(source), source);
}

static inline FFstrbuf ffStrbufCreateNWS(uint32_t length, const wchar_t* source)
{
    FFstrbuf result;
    ffStrbufInitNWS(&result, length, source);
    return result;
}

static inline FFstrbuf ffStrbufCreateWS(const wchar_t* source)
{
    if (!source) return ffStrbufCreate();
    return ffStrbufCreateNWS((uint32_t)wcslen(source), source);
}
