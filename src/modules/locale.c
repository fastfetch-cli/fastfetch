#include "fastfetch.h"
#include "common/caching.h"
#include "common/printing.h"
#include "detection/locale/locale.h"

#define FF_LOCALE_MODULE_NAME "Locale"
#define FF_LOCALE_NUM_FORMAT_ARGS 1

void ffPrintLocale(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_LOCALE_MODULE_NAME, &instance->config.locale, FF_LOCALE_NUM_FORMAT_ARGS))
        return;

    FFstrbuf locale;
    ffStrbufInit(&locale);

    ffDetectLocale(&locale);
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
