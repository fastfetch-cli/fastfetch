#include "detection/locale/locale.h"

#include <locale.h>

const char* ffDetectLocale(FFstrbuf* result)
{
    ffStrbufAppendS(result, getenv("LC_ALL"));
    if(result->length > 0)
        return NULL;

    ffStrbufAppendS(result, getenv("LANG"));
    if(result->length > 0)
        return NULL;

    ffStrbufAppendS(result, setlocale(LC_TIME, NULL));
    if(result->length > 0)
        return NULL;

    return "Failed to detect locale";
}
