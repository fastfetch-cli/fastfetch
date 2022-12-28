#pragma once

#ifndef FASTFETCH_INCLUDED_UNICODE_H
#define FASTFETCH_INCLUDED_UNICODE_H

#include "fastfetch.h"
#include <wchar.h>

void ffStrbufSetNWS(FFstrbuf* result, uint32_t length, const wchar_t* source);
static inline void ffStrbufSetWS(FFstrbuf* result, const wchar_t* source)
{
    return ffStrbufSetNWS(result, (uint32_t)wcslen(source), source);
}

FFstrbuf ffStrbufCreateNWS(uint32_t length, const wchar_t* source);
static inline FFstrbuf ffStrbufCreateWS(const wchar_t* source)
{
    return ffStrbufCreateNWS((uint32_t)wcslen(source), source);
}

#endif
