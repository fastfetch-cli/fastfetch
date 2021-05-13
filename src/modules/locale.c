#include "fastfetch.h"

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

    ffStrbufAppendS(locale, getenv("LC_CTYPE"));
    if(locale->length > 0)
        return;

    ffStrbufAppendS(locale, getenv("LC_MESSAGES"));
}

void ffPrintLocale(FFinstance* instance)
{
	if(ffPrintFromCache(instance, FF_LOCALE_MODULE_NAME, &instance->config.localeKey, &instance->config.localeFormat, FF_LOCALE_NUM_FORMAT_ARGS))
        return;

	FFstrbuf locale;
    ffStrbufInit(&locale);

    ffParsePropFile("/etc/locale.conf", "LANG=", &locale);

	if (locale.length == 0)
        getLocaleFromEnv(&locale);

    if(locale.length == 0)
    {
        ffPrintError(instance, FF_LOCALE_MODULE_NAME, 0, &instance->config.localeKey, &instance->config.localeFormat, FF_LOCALE_NUM_FORMAT_ARGS, "No locale found");
        return;
    }

    ffPrintAndSaveToCache(instance, FF_LOCALE_MODULE_NAME, &instance->config.localeKey, &locale, &instance->config.localeFormat, FF_LOCALE_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &locale}
    });

	ffStrbufDestroy(&locale);
}
