#include "fastfetch.h"
#include "common/library.h"

#include <stdarg.h>

static void* libraryLoad(const char* path, int maxVersion)
{
    void* result = dlopen(path, RTLD_LAZY);
    if(result != NULL || maxVersion < 0)
        return result;

    FFstrbuf pathbuf;
    ffStrbufInitA(&pathbuf, 64);
    ffStrbufAppendS(&pathbuf, path);
    ffStrbufAppendC(&pathbuf, '.');

    for(int i = maxVersion; i >= 0; --i)
    {
        uint32_t originalLength = pathbuf.length;
        ffStrbufAppendF(&pathbuf, "%i", i);

        result = dlopen(pathbuf.chars, RTLD_LAZY);
        if(result != NULL)
            break;

        ffStrbufSubstrBefore(&pathbuf, originalLength);
    }

    ffStrbufDestroy(&pathbuf);
    return result;
}

void* ffLibraryLoad(const FFstrbuf* userProvidedName, ...)
{
    if(userProvidedName != NULL && userProvidedName->length > 0)
        return dlopen(userProvidedName->chars, RTLD_LAZY);

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
