#include "detection/locale/locale.h"

#include <locale.h>

void ffDetectLocale(FFstrbuf* result)
{
    ffStrbufAppendS(result, getenv("LC_MESSAGES"));
    if(result->length > 0)
        return;

    ffStrbufAppendS(result, getenv("LANG"));
    if(result->length > 0)
        return;

    #ifdef LC_MESSAGES
    ffStrbufAppendS(result, setlocale(LC_MESSAGES, NULL));
    if(result->length > 0)
        return;
    #endif

    ffStrbufAppendS(result, setlocale(LC_TIME, NULL));
}
