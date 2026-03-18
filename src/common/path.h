#pragma once

#include "common/FFstrbuf.h"
#include "common/stringUtils.h"

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

#if _WIN32
char* frealpath(void* __restrict hFile, char* __restrict resolved_name /*MAX_PATH*/);
char* realpath(const char* __restrict file_name, char* __restrict resolved_name  /*MAX_PATH*/);
ssize_t freadlink(void* hFile, char* buf, size_t bufsiz);
ssize_t readlink(const char* path, char* buf, size_t bufsiz);
#endif
