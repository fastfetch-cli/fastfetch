#pragma once

#ifndef FF_INCLUDED_common_library
#define FF_INCLUDED_common_library

#include <dlfcn.h>

#ifdef __APPLE__
    #define FF_LIBRARY_EXTENSION ".dylib"
#else
    #define FF_LIBRARY_EXTENSION ".so"
#endif

#define FF_LIBRARY_SYMBOL(symbolName) \
    __typeof__(&symbolName) ff ## symbolName;

#define FF_LIBRARY_LOAD(libraryObjectName, userLibraryName, returnValue, ...) \
    void* libraryObjectName =  ffLibraryLoad(&userLibraryName, __VA_ARGS__, NULL);\
    if(libraryObjectName == NULL) \
        return returnValue;

#define FF_LIBRARY_LOAD_SYMBOL_ADRESS(library, symbolMapping, symbolName, returnValue) \
    symbolMapping = dlsym(library, #symbolName); \
    if(symbolMapping == NULL) \
    { \
        dlclose(library); \
        return returnValue; \
    }

#define FF_LIBRARY_LOAD_SYMBOL(library, symbolName, returnValue) \
    __typeof__(&symbolName) FF_LIBRARY_LOAD_SYMBOL_ADRESS(library, ff ## symbolName, symbolName, returnValue);

void* ffLibraryLoad(const FFstrbuf* userProvidedName, ...);

#endif
