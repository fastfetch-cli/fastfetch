#include "fastfetch.h"
#include "common/printing.h"
#include "detection/locale/locale.h"

#define FF_LOCALE_MODULE_NAME "Locale"
#define FF_LOCALE_NUM_FORMAT_ARGS 1

void ffPrintLocale(FFinstance* instance)
{
    FFstrbuf locale;
    ffStrbufInit(&locale);

    ffDetectLocale(&locale);
    if(locale.length == 0)
    {
        ffPrintError(instance, FF_LOCALE_MODULE_NAME, 0, &instance->config.locale, "No locale found");
        return;
    }

    if(instance->config.locale.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_LOCALE_MODULE_NAME, 0, &instance->config.locale.key);
        ffStrbufPutTo(&locale, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_LOCALE_MODULE_NAME, 0, &instance->config.locale, FF_LOCALE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &locale}
        });
    }

    ffStrbufDestroy(&locale);
}
