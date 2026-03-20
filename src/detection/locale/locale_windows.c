#include "detection/locale/locale.h"
#include "common/windows/unicode.h"

#include <windows.h>

const char* ffDetectLocale(FFstrbuf* result)
{
    wchar_t name[LOCALE_NAME_MAX_LENGTH];
    int size = GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH);
    if (size <= 1) // including '\0'
        return "GetUserDefaultLocaleName() failed";

    ffStrbufSetNWS(result, (uint32_t)size - 1, name);

    return NULL;
}
