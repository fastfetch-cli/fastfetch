#pragma once

#include "fastfetch.h"
#include "util/FFcheckmacros.h"

#ifndef FF_DISABLE_DLOPEN

#if defined(_WIN32)
    #include <libloaderapi.h>
    #define FF_DLOPEN_FLAGS 0
    FF_C_NODISCARD static inline void* dlopen(const char* path, int mode) { FF_UNUSED(mode); return LoadLibraryA(path); }
    FF_C_NODISCARD static inline void* dlsym(void* handle, const char* symbol) { return (void*) GetProcAddress((HMODULE)handle, symbol); }
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

static inline void ffLibraryUnload(void** handle)
{
    assert(handle);
    if (*handle)
        dlclose(*handle);
}

#define FF_LIBRARY_SYMBOL(symbolName) \
    __typeof__(&symbolName) ff ## symbolName;

#define FF_LIBRARY_LOAD(libraryObjectName, userLibraryName, returnValue, ...) \
    void* __attribute__((__cleanup__(ffLibraryUnload))) libraryObjectName = ffLibraryLoad(userLibraryName, __VA_ARGS__, NULL);\
    if(libraryObjectName == NULL) \
        return returnValue;

#define FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, symbolMapping, symbolName, returnValue) \
    symbolMapping = (__typeof__(&symbolName)) dlsym(library, #symbolName); \
    if(symbolMapping == NULL) \
        return returnValue;

#define FF_LIBRARY_LOAD_SYMBOL_ADDRESS2(library, symbolMapping, symbolName, alternateName, returnValue) \
    symbolMapping = dlsym(library, #symbolName); \
    if(symbolMapping == NULL && !(symbolMapping = (__typeof__(&symbolName)) dlsym(library, #alternateName))) \
        return returnValue;

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
    __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff ## symbolName, symbolName, returnValue);

#define FF_LIBRARY_LOAD_SYMBOL_LAZY(library, symbolName) \
    __typeof__(&symbolName) ff ## symbolName = (__typeof__(&symbolName)) dlsym(library, #symbolName);

#define FF_LIBRARY_LOAD_SYMBOL_MESSAGE(library, symbolName) \
    __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff ## symbolName, symbolName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_MESSAGE2(library, symbolName, alternateName) \
    __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADDRESS2(library, ff ## symbolName, symbolName, alternateName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_VAR(library, varName, symbolName, returnValue) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff ## symbolName, symbolName, returnValue);

#define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(library, varName, symbolName) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff ## symbolName, symbolName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE2(library, varName, symbolName, alternateName) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS2(library, (varName).ff ## symbolName, symbolName, alternateName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_PTR(library, varName, symbolName, returnValue) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName)->ff ## symbolName, symbolName, returnValue);

void* ffLibraryLoad(const FFstrbuf* userProvidedName, ...);

#else

#define FF_LIBRARY_EXTENSION ""

#define FF_LIBRARY_SYMBOL(symbolName) \
    __typeof__(&symbolName) ff ## symbolName;

#define FF_LIBRARY_LOAD(libraryObjectName, userLibraryName, returnValue, ...) \
    FF_MAYBE_UNUSED void* libraryObjectName = NULL; // Placeholder

#define FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, symbolMapping, symbolName, returnValue) \
    symbolMapping = (__typeof__(&symbolName)) &symbolName

#define FF_LIBRARY_LOAD_SYMBOL_ADDRESS2(library, symbolMapping, symbolName, alternateName, returnValue) \
    symbolMapping = (__typeof__(&symbolName)) &symbolName

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
    FF_MAYBE_UNUSED __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff ## symbolName, symbolName, returnValue);

#define FF_LIBRARY_LOAD_SYMBOL_LAZY(library, symbolName) \
    FF_MAYBE_UNUSED __typeof__(&symbolName) ff ## symbolName = (__typeof__(&symbolName)) &symbolName;

#define FF_LIBRARY_LOAD_SYMBOL_MESSAGE(library, symbolName) \
    FF_MAYBE_UNUSED __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, ff ## symbolName, symbolName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_MESSAGE2(library, symbolName, alternateName) \
    FF_MAYBE_UNUSED __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADDRESS2(library, ff ## symbolName, symbolName, alternateName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_VAR(library, varName, symbolName, returnValue) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff ## symbolName, symbolName, returnValue);

#define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(library, varName, symbolName) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName).ff ## symbolName, symbolName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE2(library, varName, symbolName, alternateName) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS2(library, (varName).ff ## symbolName, symbolName, alternateName, "dlsym " #symbolName " failed");

#define FF_LIBRARY_LOAD_SYMBOL_PTR(library, varName, symbolName, returnValue) \
    FF_LIBRARY_LOAD_SYMBOL_ADDRESS(library, (varName)->ff ## symbolName, symbolName, returnValue);

#endif
