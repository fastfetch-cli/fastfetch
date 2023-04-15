#include "fastfetch.h"
#include "common/printing.h"
#include "detection/wmtheme/wmtheme.h"

#define FF_WMTHEME_MODULE_NAME "WM Theme"
#define FF_WMTHEME_NUM_FORMAT_ARGS 1

void ffPrintWMTheme(FFinstance* instance)
{
    FF_STRBUF_AUTO_DESTROY themeOrError = ffStrbufCreate();
    if(ffDetectWmTheme(instance, &themeOrError))
    {
        if(instance->config.wmTheme.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme.key);
            puts(themeOrError.chars);
        }
        else
        {
            ffPrintFormat(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, FF_WMTHEME_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &themeOrError}
            });
        }
    }
    else
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmTheme, "%*s", themeOrError.length, themeOrError.chars);
    }
}
