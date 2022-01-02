#include "fastfetch.h"

#define FF_WM_MODULE_NAME "WM"
#define FF_WM_NUM_FORMAT_ARGS 3

void ffPrintWM(FFinstance* instance)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer(instance);

    if(result->wmPrettyName.length == 0)
    {
        ffPrintError(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, FF_WM_NUM_FORMAT_ARGS, "No WM found");
        return;
    }

    if(instance->config.wmFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey);

        if(result->wmPrettyName.length == 0 && result->wmProcessName.length == 0)
        {
            ffStrbufPutTo(&result->wmProtocolName, stdout);
        }
        else
        {
            if(result->wmPrettyName.length > 0)
                ffStrbufWriteTo(&result->wmPrettyName, stdout);
            else
                ffStrbufWriteTo(&result->wmProcessName, stdout);

            if(result->wmProtocolName.length > 0)
            {
                fputs(" (", stdout);
                ffStrbufWriteTo(&result->wmProtocolName, stdout);
                putchar(')');
            }

            putchar('\n');
        }
    }
    else
    {
        ffPrintFormatString(instance, FF_WM_MODULE_NAME, 0, &instance->config.wmKey, &instance->config.wmFormat, NULL, FF_WM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmProcessName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmPrettyName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmProtocolName}
        });
    }
}
