#include "detection/locale/locale.h"
#include "util/windows/unicode.h"

#include <windows.h>
#include <locale.h>

void ffDetectLocale(FFstrbuf* result)
{
    wchar_t name[LOCALE_NAME_MAX_LENGTH];
    int size = GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH);
    if (size > 1) // including '\0'
        ffStrbufSetNWS(result, (uint32_t)size - 1, name);
}
