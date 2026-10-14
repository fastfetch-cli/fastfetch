#pragma once

#ifndef NDEBUG
    #include "fastfetch.h"
    #include "common/time.h"

[[gnu::nonnull(1, 3), gnu::format(printf, 3, 4)]]
void ffDebugPrint(const char* file, int line, const char* format, ...);

    #define FF_DEBUG(format, ...) ffDebugPrint(__FILE__, __LINE__, format, ##__VA_ARGS__)

    #if _WIN32
const char* ffDebugWin32Error(DWORD errorCode);
const char* ffDebugNtStatus(NTSTATUS status);
const char* ffDebugConfigRet(unsigned long /*CONFIGRET*/ ret);
const char* ffDebugHResult(HRESULT hr);
    #endif

#else

    #define FF_DEBUG(format, ...) ((void) 0)
#endif
