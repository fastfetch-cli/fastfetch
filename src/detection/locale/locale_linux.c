#include "detection/locale/locale.h"

#include <locale.h>

void ffDetectLocale(FFstrbuf* result)
{
    ffStrbufAppendS(result, getenv("LC_ALL"));
    if(result->length > 0)
        return;

    ffStrbufAppendS(result, getenv("LANG"));
    if(result->length > 0)
        return;

    ffStrbufAppendS(result, setlocale(LC_TIME, NULL));
}
