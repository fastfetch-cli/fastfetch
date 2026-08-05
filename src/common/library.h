#pragma once

#include "fastfetch.h"

#ifndef FF_DISABLE_DLOPEN

    #if defined(_WIN32)
        #define FF_DLOPEN_FLAGS 0
[[nodiscard]] void* dlopen(const char* path, int mode);
[[nodiscard]] void* dlsym(void* handle, const char* symbol);
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

    #define FF_LIBRARY_SYMBOL(symbolName) \
        typeof(&symbolName) ff##symbolName;

    #define FF_LIBRARY_LOAD(libraryObjectName, returnValue, libraryFileName, maxVersion, ...)                                          \
        [[gnu::cleanup(ffLibraryUnload)]] void* libraryObjectName = ffLibraryLoadSingle(libraryFileName, maxVersion);                      \
        __VA_OPT__(if (__builtin_expect(libraryObjectName == nullptr, false)) libraryObjectName = ffLibraryLoadMulti(__VA_ARGS__, nullptr);) \
        if (__builtin_expect(libraryObjectName == nullptr, false))                                                                        \
            return returnValue;

    #define FF_LIBRARY_LOAD_MESSAGE(libraryObjectName, libraryFileName, maxVersion, ...) \
        FF_LIBRARY_LOAD(libraryObjectName, "dlopen(" libraryFileName ") failed", libraryFileName, maxVersion, ##__VA_ARGS__)

    #define FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, symbolMapping, symbolName, returnValue) \
        symbolMapping = (typeof(&symbolName)) dlsym(library, #symbolName);              \
        if (__builtin_expect(symbolMapping == nullptr, false))                                 \
            return returnValue;

    #define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
        auto FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff##symbolName, symbolName, returnValue);

    #define FF_LIBRARY_LOAD_SYMBOL_LAZY(library, symbolName) \
        auto ff##symbolName = (typeof(&symbolName)) dlsym(library, #symbolName);

    #define FF_LIBRARY_LOAD_SYMBOL_MESSAGE(library, symbolName) \
        auto FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff##symbolName, symbolName, "dlsym " #symbolName " failed");

    #define FF_LIBRARY_LOAD_SYMBOL_VAR(library, varName, symbolName, returnValue) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff##symbolName, symbolName, returnValue);

    #define FF_LIBRARY_LOAD_SYMBOL_VAR_LAZY(library, varName, symbolName) \
        (varName).ff##symbolName = (typeof(&symbolName)) dlsym(library, #symbolName);

    #define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(library, varName, symbolName) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff##symbolName, symbolName, "dlsym " #symbolName " failed");

    #define FF_LIBRARY_LOAD_SYMBOL_PTR(library, varName, symbolName, returnValue) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName)->ff##symbolName, symbolName, returnValue);

void* ffLibraryLoadSingle(const char* path, int maxVersion);
void* ffLibraryLoadMulti(const char* path, int maxVersion, ...);

#else

    #define FF_LIBRARY_EXTENSION ""

    #define FF_LIBRARY_SYMBOL(symbolName) \
        typeof(&symbolName) ff##symbolName;

    #define FF_LIBRARY_LOAD(libraryObjectName, returnValue, ...) \
        [[maybe_unused]] void* libraryObjectName = nullptr; // Placeholder

    #define FF_LIBRARY_LOAD_MESSAGE(libraryObjectName, libraryFileName, maxVersion, ...) \
        FF_LIBRARY_LOAD(libraryObjectName, , libraryFileName, maxVersion, ##__VA_ARGS__)

    #define FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, symbolMapping, symbolName, returnValue) \
        symbolMapping = (typeof(&symbolName)) &symbolName;

    #define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
        [[maybe_unused]] auto FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff##symbolName, symbolName, returnValue);

    #define FF_LIBRARY_LOAD_SYMBOL_LAZY(library, symbolName) \
        [[maybe_unused]] auto ff##symbolName = (typeof(&symbolName)) &symbolName;

    #define FF_LIBRARY_LOAD_SYMBOL_MESSAGE(library, symbolName) \
        [[maybe_unused]] auto FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff##symbolName, symbolName, "dlsym " #symbolName " failed");

    #define FF_LIBRARY_LOAD_SYMBOL_VAR(library, varName, symbolName, returnValue) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff##symbolName, symbolName, returnValue);

    #define FF_LIBRARY_LOAD_SYMBOL_VAR_LAZY(library, varName, symbolName) \
        (varName).ff##symbolName = (typeof(&symbolName)) &symbolName;

    #define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(library, varName, symbolName) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff##symbolName, symbolName, "dlsym " #symbolName " failed");

    #define FF_LIBRARY_LOAD_SYMBOL_PTR(library, varName, symbolName, returnValue) \
        FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName)->ff##symbolName, symbolName, returnValue);

#endif

#if _WIN32
void* ffLibraryGetModule(const wchar_t* libraryFileName);
#endif
