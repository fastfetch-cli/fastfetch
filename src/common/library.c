#include "fastfetch.h"

#include <stdarg.h>

//common/libraries.c
void* ffLibraryLoad(const FFstrbuf* userProvidedName, ...)
{
    if(userProvidedName->length > 0)
        return dlopen(userProvidedName->chars, RTLD_LAZY);

    va_list defaultNames;
    va_start(defaultNames, userProvidedName);

    void* result = NULL;

    while(result == NULL)
    {
        const char* name = va_arg(defaultNames, const char*);
        if(name == NULL)
            break;
        result = dlopen(name, RTLD_LAZY);
    }

    va_end(defaultNames);

    return result;
}
