#pragma once

#include "common/FFstrbuf.h"
#include "common/strutil.h"
#include "fastfetch_config.h"

const char* ffFindExecutableInPath(const char* name, FFstrbuf* result);
static inline bool ffIsAbsolutePath(const char* path) {
#ifdef _WIN32
    return (ffCharIsEnglishAlphabet(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) // drive letter path
        || (path[0] == '\\' && path[1] == '\\');                                                       // UNC path
#else
    return path[0] == '/';
#endif
}

#if _WIN32
char* frealpath(void* __restrict hFile, char* __restrict resolved_name /*MAX_PATH*/);
char* realpath(const char* __restrict file_name, char* __restrict resolved_name /*MAX_PATH*/);
ssize_t freadlink(void* hFile, char* buf, size_t bufsiz);
ssize_t readlink(const char* path, char* buf, size_t bufsiz);
#endif

#ifdef __FreeBSD__
    #include <paths.h>
    #ifdef _PATH_LOCALBASE
        #define FF_PATH_PKG_BASE _PATH_LOCALBASE
    #else
        #define FF_PATH_PKG_BASE "/usr/local"
    #endif
#elif __OpenBSD__
    #define FF_PATH_PKG_BASE "/usr/local"
#elif __NetBSD__
    #define FF_PATH_PKG_BASE "/usr/pkg"
#else
    #define FF_PATH_PKG_BASE FASTFETCH_TARGET_DIR_USR
#endif