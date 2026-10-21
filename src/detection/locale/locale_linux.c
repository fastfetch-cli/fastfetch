#include "detection/locale/locale.h"
#include "common/debug.h"

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

    FF_DEBUG("LC_ALL, LANG and setlocale(LC_TIME, nullptr) are all empty");
    return "Failed to detect locale";
}
