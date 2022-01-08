#include "fastfetch.h"

#include <stdarg.h>

void* ffLibraryLoad(const FFstrbuf* userProvidedName, ...)
{
    if(userProvidedName->length > 0)
        return dlopen(userProvidedName->chars, RTLD_LAZY);

    va_list defaultNames;
    va_start(defaultNames, userProvidedName);

    void* result = NULL;
    const char* name = va_arg(defaultNames, const char*);

    while(result == NULL && name != NULL)
    {
        result = dlopen(name, RTLD_LAZY);
        name = va_arg(defaultNames, const char*);
    }

    va_end(defaultNames);

    return result;
}
