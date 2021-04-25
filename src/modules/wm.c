#include "fastfetch.h"

#define FF_WM_MODULE_NAME "WM"
#define FF_WM_NUM_FORMAT_ARGS 3

void ffPrintWM(FFinstance* instance)
{
    const FFWMResult* result = ffCalculateWM(instance);

    if(result->error.length > 0)
    {
        ffPrintError(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, FF_WM_NUM_FORMAT_ARGS, result->error.chars);
        return;
    }

    if(instance->config.wmFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, "WM", 0, &instance->config.wmKey);

        if(result->prettyName.length == 0 && result->processName.length == 0)
        {
            ffStrbufPutTo(&result->protocolName, stdout);
        }
        else
        {
            if(result->prettyName.length > 0)
                ffStrbufWriteTo(&result->prettyName, stdout);
            else
                ffStrbufWriteTo(&result->processName, stdout);

            if(result->protocolName.length > 0)
            {
                fputs(" (", stdout);
                ffStrbufWriteTo(&result->protocolName, stdout);
                putchar(')');
            }

            putchar('\n');
        }
    }
    else
    {
        ffPrintFormatString(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, NULL, FF_WM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->processName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->prettyName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->protocolName}
        });
    }
}
