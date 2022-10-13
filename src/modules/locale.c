#include "fastfetch.h"
#include "common/properties.h"
#include "common/printing.h"
#include "common/caching.h"
#include "common/parsing.h"

#include <stdlib.h>
#include <locale.h>

#define FF_LOCALE_MODULE_NAME "Locale"
#define FF_LOCALE_NUM_FORMAT_ARGS 1

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

static void getLocaleFromCmd(FFstrbuf* locale)
{
    ffStrbufAppendS(locale, setlocale(LC_ALL, NULL));

    #ifdef LC_MESSAGES
    if(locale->length > 0)
        return;

    ffStrbufAppendS(locale, setlocale(LC_MESSAGES, NULL));
    #endif
}

void ffPrintLocale(FFinstance* instance)
{
	if(ffPrintFromCache(instance, FF_LOCALE_MODULE_NAME, &instance->config.locale, FF_LOCALE_NUM_FORMAT_ARGS))
        return;

	FFstrbuf locale;
    ffStrbufInit(&locale);

    //Ubuntu (and deriviates) use a non standard locale file.
    //Parse it first, because on distributions where it exists, it takes precedence.
    //Otherwise use the standard etc/locale.conf file.
    ffParsePropFile(FASTFETCH_TARGET_DIR_ETC"/default/locale", "LANG =", &locale);

    if(locale.length == 0)
    {
        ffParsePropFile(FASTFETCH_TARGET_DIR_ETC"/locale.conf", "LANG =", &locale);
    }

	if(locale.length == 0)
    {
        getLocaleFromEnv(&locale);
    }

    if(locale.length == 0)
    {
        getLocaleFromCmd(&locale);
    }

    if(locale.length == 0)
    {
        ffPrintError(instance, FF_LOCALE_MODULE_NAME, 0, &instance->config.locale, "No locale found");
        return;
    }

    ffPrintAndWriteToCache(instance, FF_LOCALE_MODULE_NAME, &instance->config.locale, &locale, FF_LOCALE_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &locale}
    });

	ffStrbufDestroy(&locale);
}
