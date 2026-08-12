#include "detection/locale/locale.h"
#include "common/windows/unicode.h"

#include <winnls.h>

const char* ffDetectLocale(FFstrbuf* result) {
    ffStrbufAppendS(result, getenv("LC_ALL"));
    if (result->length > 0) {
        return nullptr;
    }

    ffStrbufAppendS(result, getenv("LANG")); // Available in MSYS2 and Cygwin
    if (result->length > 0) {
        return nullptr;
    }

    wchar_t name[LOCALE_NAME_MAX_LENGTH];
    int size = GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH);
    if (size <= 1) { // including '\0'
        return "GetUserDefaultLocaleName() failed";
    }

    ffStrbufSetNWS(result, (uint32_t) size - 1, name);

    return nullptr;
}
