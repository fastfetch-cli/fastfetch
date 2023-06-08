#include "fastfetch.h"
#include "common/printing.h"
#include "detection/theme/theme.h"

#define FF_THEME_MODULE_NAME "Theme"
#define FF_THEME_NUM_FORMAT_ARGS 1

void ffPrintTheme(FFinstance* instance)
{
    FF_STRBUF_AUTO_DESTROY theme;
    ffStrbufInit(&theme);
    const char* error = ffDetectTheme(instance, &theme);
    if (error)
    {
        ffPrintError(instance, FF_THEME_MODULE_NAME, 0, &instance->config.theme, "%s", error);
        return;
    }

    if(instance->config.theme.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_THEME_MODULE_NAME, 0, &instance->config.theme.key);
        ffStrbufPutTo(&theme, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_THEME_MODULE_NAME, 0, &instance->config.theme, FF_THEME_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &theme}
        });
    }
}
