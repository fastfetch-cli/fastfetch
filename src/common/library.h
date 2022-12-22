#pragma once

#ifndef FF_INCLUDED_common_library
#define FF_INCLUDED_common_library

#include "fastfetch.h"
#include "util/FFcheckmacros.h"

#if defined(_WIN32) //We don't force MSYS using LoadLibrary because dlopen also searches $LD_LIBRARY_PATH
    #include <libloaderapi.h>
    #define FF_DLOPEN_FLAGS 0
    FF_C_NODISCARD static inline void* dlopen(const char* path, int mode) { FF_UNUSED(mode); return LoadLibraryA(path); }
    FF_C_NODISCARD static inline void* dlsym(void* handle, const char* symbol) { return GetProcAddress((HMODULE)handle, symbol); }
    static inline int dlclose(void* handle) { return !FreeLibrary((HMODULE)handle); }
#else
    #include <dlfcn.h>
#endif

#ifdef _WIN32
    #define FF_LIBRARY_EXTENSION ".dll"
#elif defined(__APPLE__)
    #define FF_LIBRARY_EXTENSION ".dylib"
#else
    #define FF_LIBRARY_EXTENSION ".so"
#endif

#define FF_LIBRARY_SYMBOL(symbolName) \
    __typeof__(&symbolName) ff ## symbolName;

#define FF_LIBRARY_LOAD(libraryObjectName, userLibraryName, returnValue, ...) \
    void* libraryObjectName = ffLibraryLoad(userLibraryName, __VA_ARGS__, NULL);\
    if(libraryObjectName == NULL) \
        return returnValue;

#define FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, symbolMapping, symbolName, returnValue) \
    symbolMapping = dlsym(library, #symbolName); \
    if(symbolMapping == NULL) \
    { \
        dlclose(library); \
        return returnValue; \
    }

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
    __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff ## symbolName, symbolName, returnValue);

#define FF_LIBRARY_LOAD_SYMBOL_LAZY(library, symbolName) \
    __typeof__(&symbolName) ff ## symbolName = dlsym(library, #symbolName);

#define FF_LIBRARY_LOAD_SYMBOL_MESSAGE(library, symbolName) \
    __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff ## symbolName, symbolName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_VAR(library, varName, symbolName, returnValue) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, varName.ff ## symbolName, symbolName, returnValue);

#define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(library, varName, symbolName) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, varName.ff ## symbolName, symbolName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_PTR(library, varName, symbolName, returnValue) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, varName->ff ## symbolName, symbolName, returnValue);

void* ffLibraryLoad(const FFstrbuf* userProvidedName, ...);

#endif
