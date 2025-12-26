#pragma once

#include "util/FFstrbuf.h"
#include "util/stringUtils.h"

const char* ffFindExecutableInPath(const char* name, FFstrbuf* result);
static inline bool ffIsAbsolutePath(const char* path)
{
    #ifdef _WIN32
    return (ffCharIsEnglishAlphabet(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) // drive letter path
        || (path[0] == '\\' && path[1] == '\\'); // UNC path
    #else
    return path[0] == '/';
    #endif
}
