#pragma once

#include "fastfetch.h"

#ifndef FF_DISABLE_DLOPEN

    #if defined(_WIN32)
        #define FF_DLOPEN_FLAGS 0
FF_A_NODISCARD void* dlopen(const char* path, int mode);
FF_A_NODISCARD void* dlsym(void* handle, const char* symbol);
int dlclose(void* handle);
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

static inline void ffLibraryUnload(void** handle) {
    assert(handle);
    if (*handle) {
        dlclose(*handle);
    }
}

    #if __cplusplus
        #define __auto_type auto
    #endif

    #define FF_LIBRARY_SYMBOL(symbolName) \
        __typeof__(&symbolName) ff##symbolName;

    #define FF_LIBRARY_LOAD(libraryObjectName, returnValue, ...)                                  \
        void* FF_A_CLEANUP(ffLibraryUnload) libraryObjectName = ffLibraryLoad(__VA_ARGS__, NULL); \
        if (libraryObjectName == NULL)                                                            \
            return returnValue;

    #define FF_LIBRARY_LOAD_MESSAGE(libraryObjectName, libraryFileName, maxVersion, ...) \
        FF_LIBRARY_LOAD(libraryObjectName, "dlopen(" libraryFileName ") failed", libraryFileName, maxVersion, ##__VA_ARGS__)

    #define FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, symbolMapping, symbolName, returnValue) \
        symbolMapping = (__typeof__(&symbolName)) dlsym(library, #symbolName);              \
        if (symbolMapping == NULL)                                                          \
            return returnValue;

    #define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
        __auto_type FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff##symbolName, symbolName, returnValue);

    #define FF_LIBRARY_LOAD_SYMBOL_LAZY(library, symbolName) \
        __auto_type ff##symbolName = (__typeof__(&symbolName)) dlsym(library, #symbolName);

    #define FF_LIBRARY_LOAD_SYMBOL_MESSAGE(library, symbolName) \
        __auto_type FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff##symbolName, symbolName, "dlsym " #symbolName " failed");

    #define FF_LIBRARY_LOAD_SYMBOL_VAR(library, varName, symbolName, returnValue) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff##symbolName, symbolName, returnValue);

    #define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(library, varName, symbolName) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff##symbolName, symbolName, "dlsym " #symbolName " failed");

    #define FF_LIBRARY_LOAD_SYMBOL_PTR(library, varName, symbolName, returnValue) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName)->ff##symbolName, symbolName, returnValue);

void* ffLibraryLoad(const char* path, int maxVersion, ...);

#else

    #define FF_LIBRARY_EXTENSION ""

    #define FF_LIBRARY_SYMBOL(symbolName) \
        __typeof__(&symbolName) ff##symbolName;

    #define FF_LIBRARY_LOAD(libraryObjectName, returnValue, ...) \
        FF_A_UNUSED void* libraryObjectName = NULL; // Placeholder

    #define FF_LIBRARY_LOAD_MESSAGE(libraryObjectName, libraryFileName, maxVersion, ...) \
        FF_LIBRARY_LOAD(libraryObjectName, , libraryFileName, maxVersion, ##__VA_ARGS__)

    #define FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, symbolMapping, symbolName, returnValue) \
        symbolMapping = (__typeof__(&symbolName)) &symbolName;

    #define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
        FF_A_UNUSED __auto_type FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff##symbolName, symbolName, returnValue);

    #define FF_LIBRARY_LOAD_SYMBOL_LAZY(library, symbolName) \
        FF_A_UNUSED __auto_type ff##symbolName = (__typeof__(&symbolName)) &symbolName;

    #define FF_LIBRARY_LOAD_SYMBOL_MESSAGE(library, symbolName) \
        FF_A_UNUSED __auto_type FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff##symbolName, symbolName, "dlsym " #symbolName " failed");

    #define FF_LIBRARY_LOAD_SYMBOL_VAR(library, varName, symbolName, returnValue) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff##symbolName, symbolName, returnValue);

    #define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(library, varName, symbolName) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff##symbolName, symbolName, "dlsym " #symbolName " failed");

    #define FF_LIBRARY_LOAD_SYMBOL_PTR(library, varName, symbolName, returnValue) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName)->ff##symbolName, symbolName, returnValue);

#endif

#if _WIN32
void* ffLibraryGetModule(const wchar_t* libraryFileName);
#endif
