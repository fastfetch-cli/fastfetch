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

    setlocale(LC_ALL, "");
    #ifdef LC_MESSAGES
    ffStrbufAppendS(result, setlocale(LC_MESSAGES, NULL));

    if(result->length == 0)
        ffStrbufAppendS(result, setlocale(LC_ALL, NULL));
    #endif
    setlocale(LC_ALL, "C");
}
