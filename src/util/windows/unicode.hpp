#pragma once

#ifndef FASTFETCH_INCLUDED_UNICODE_HPP
#define FASTFETCH_INCLUDED_UNICODE_HPP

#ifdef __cplusplus

extern "C" {
#include "unicode.h"
}

#include <string_view>

static inline void ffStrbufSetWSV(FFstrbuf* result, const std::wstring_view source)
{
    return ffStrbufSetNWS(result, (uint32_t) source.size(), source.data());
}

#else

    #error Must be included in C++ source file

#endif

#endif
