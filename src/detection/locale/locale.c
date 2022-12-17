#include "detection/locale/locale.h"

#include "common/properties.h"
#include "common/parsing.h"
#include <stdlib.h>
#include <locale.h>

__attribute__((__unused__))
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
    ffStrbufAppendS(locale, setlocale(LC_ALL, NULL));

    #ifdef LC_MESSAGES
    if(locale->length > 0)
        return;

    ffStrbufAppendS(locale, setlocale(LC_MESSAGES, NULL));
    #endif
}

void ffDetectLocale(FFstrbuf* result)
{
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

    #ifndef _WIN32

        getLocaleFromEnv(result);
        if(result->length > 0)
            return;

    #endif

    getLocaleFromStdFn(result);
}
