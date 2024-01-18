#include "detection/locale/locale.h"

#include "common/properties.h"
#include "common/parsing.h"
#include <stdlib.h>
#include <locale.h>

FF_MAYBE_UNUSED
static void getLocaleFromEnv(FFstrbuf* locale)
{
    ffStrbufAppendS(locale, getenv("LANG"));
    if(locale->length > 0)
        return;

    ffStrbufAppendS(locale, getenv("LC_ALL"));
    if(locale->length > 0)
        return;

    ffStrbufAppendS(locale, getenv("LC_MESSAGES"));
}

static void getLocaleFromStdFn(FFstrbuf* locale)
{
    #ifdef LC_MESSAGES
    ffStrbufAppendS(locale, setlocale(LC_MESSAGES, NULL));

    if(locale->length > 0)
        return;
    #endif

    ffStrbufAppendS(locale, setlocale(LC_ALL, NULL));
}

void ffDetectLocale(FFstrbuf* result)
{
    #ifndef _WIN32

        getLocaleFromEnv(result);
        if(result->length > 0)
            return;

    #endif

    #if !(defined(__APPLE__) || defined(_WIN32))

        //Ubuntu (and deriviates) use a non standard locale file.
        //Parse it first, because on distributions where it exists, it takes precedence.
        //Otherwise use the standard etc/locale.conf file.
        ffParsePropFile(FASTFETCH_TARGET_DIR_ETC"/default/locale", "LANG =", result);

        if(result->length > 0)
            return;

        ffParsePropFile(FASTFETCH_TARGET_DIR_ETC"/locale.conf", "LANG =", result);
        if(result->length > 0)
            return;

    #endif

    getLocaleFromStdFn(result);
}
