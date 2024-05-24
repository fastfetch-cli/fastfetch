#include "fastfetch.h"
#include "common/library.h"

#ifndef FF_DISABLE_DLOPEN

#include <stdarg.h>

//Clang doesn't define __SANITIZE_ADDRESS__ but defines __has_feature(address_sanitizer)
#if !defined(__SANITIZE_ADDRESS__) && defined(__has_feature)
    #if __has_feature(address_sanitizer)
        #define __SANITIZE_ADDRESS__
    #endif
#endif

#ifndef FF_DLOPEN_FLAGS
    #ifdef __SANITIZE_ADDRESS__
        #define FF_DLOPEN_FLAGS RTLD_LAZY | RTLD_NODELETE
    #else
        #define FF_DLOPEN_FLAGS RTLD_LAZY
    #endif
#endif

static void* libraryLoad(const char* path, int maxVersion)
{
    void* result = dlopen(path, FF_DLOPEN_FLAGS);

    #ifdef _WIN32

    // libX.dll.1 never exists on Windows, while libX-1.dll may exist
    FF_UNUSED(maxVersion)

    #else

    if(result != NULL || maxVersion < 0)
        return result;

    FF_STRBUF_AUTO_DESTROY pathbuf = ffStrbufCreateA(64);
    ffStrbufAppendS(&pathbuf, path);
    ffStrbufAppendC(&pathbuf, '.');

    for(int i = maxVersion; i >= 0; --i)
    {
        uint32_t originalLength = pathbuf.length;
        ffStrbufAppendF(&pathbuf, "%i", i);

        result = dlopen(pathbuf.chars, FF_DLOPEN_FLAGS);
        if(result != NULL)
            break;

        ffStrbufSubstrBefore(&pathbuf, originalLength);
    }

    #endif

    return result;
}

void* ffLibraryLoad(const FFstrbuf* userProvidedName, ...)
{
    if(userProvidedName != NULL && userProvidedName->length > 0)
        return dlopen(userProvidedName->chars, FF_DLOPEN_FLAGS);

    va_list defaultNames;
    va_start(defaultNames, userProvidedName);

    void* result = NULL;

    while(result == NULL)
    {
        const char* path = va_arg(defaultNames, const char*);
        if(path == NULL)
            break;

        int maxVersion = va_arg(defaultNames, int);
        result = libraryLoad(path, maxVersion);
    }

    va_end(defaultNames);

    return result;
}

#endif
