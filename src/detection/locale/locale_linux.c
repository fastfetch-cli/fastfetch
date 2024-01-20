#include "detection/locale/locale.h"

#include <locale.h>

void ffDetectLocale(FFstrbuf* result)
{
    #ifdef LC_MESSAGES
    ffStrbufAppendS(result, setlocale(LC_MESSAGES, NULL));

    if(result->length > 0)
        return;
    #endif

    ffStrbufAppendS(result, setlocale(LC_ALL, NULL));
}
