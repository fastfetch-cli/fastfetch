#pragma once

#include "fastfetch.h"
#include "common/time.h"

static inline const char* ffFindFileName(const char* file)
{
    const char* lastSlash = __builtin_strrchr(file, '/');
    #ifdef _WIN32
    if (lastSlash == NULL)
        lastSlash = __builtin_strrchr(file, '\\');
    #endif
    if (lastSlash != NULL)
        return lastSlash + 1;
    return file;
}

#ifndef NDEBUG
    #define FF_DEBUG_PRINT(file_, line_, format_, ...) \
        do { \
            if (instance.config.display.debugMode) \
                fprintf(stderr, "[%s%4d, %s] " format_ "\n", ffFindFileName(file_), line_, ffTimeToTimeStr(ffTimeGetNow()), ##__VA_ARGS__); \
        } while (0)
#else
    #define FF_DEBUG_PRINT(file_, line_, format_, ...) \
        do { } while (0)
#endif

#define FF_DEBUG(format, ...) FF_DEBUG_PRINT(__FILE__, __LINE__, format, ##__VA_ARGS__)

const char* ffDebugWin32Error(unsigned long errorCode);
