#include "detection/locale/locale.h"

#include <locale.h>

const char* ffDetectLocale(FFstrbuf* result) {
    ffStrbufAppendS(result, getenv("LC_ALL"));
    if (result->length > 0) {
        return nullptr;
    }

    ffStrbufAppendS(result, getenv("LANG"));
    if (result->length > 0) {
        return nullptr;
    }

    ffStrbufAppendS(result, setlocale(LC_TIME, nullptr));
    if (result->length > 0) {
        return nullptr;
    }

    return "Failed to detect locale";
}
