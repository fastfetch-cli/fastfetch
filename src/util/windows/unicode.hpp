#pragma once

#ifdef __cplusplus

extern "C" {
#include "unicode.h"
}

#include <string_view>

static inline void ffStrbufInitWSV(FFstrbuf* result, const std::wstring_view source)
{
    return ffStrbufInitNWS(result, (uint32_t) source.size(), source.data());
}

static inline void ffStrbufSetWSV(FFstrbuf* result, const std::wstring_view source)
{
    return ffStrbufSetNWS(result, (uint32_t) source.size(), source.data());
}

#else

    #error Must be included in C++ source file

#endif
