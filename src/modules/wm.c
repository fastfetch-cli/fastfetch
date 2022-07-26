#include "fastfetch.h"
#include "common/printing.h"
#include "detection/displayserver.h"

#define FF_WM_MODULE_NAME "WM"
#define FF_WM_NUM_FORMAT_ARGS 3

void ffPrintWM(FFinstance* instance)
{
    #ifdef __ANDROID__
        ffPrintError(instance, FF_WM_MODULE_NAME, 0, &instance->config.wm, "WM detection is not supported on Android");
        return;
    #endif

    const FFDisplayServerResult* result = ffConnectDisplayServer(instance);

    if(result->wmPrettyName.length == 0)
    {
        ffPrintError(instance, FF_WM_MODULE_NAME, 0, &instance->config.wm, "No WM found");
        return;
    }

    if(instance->config.wm.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WM_MODULE_NAME, 0, &instance->config.wm.key);

        ffStrbufWriteTo(&result->wmPrettyName, stdout);

        if(result->wmProtocolName.length > 0)
        {
            fputs(" (", stdout);
            ffStrbufWriteTo(&result->wmProtocolName, stdout);
            putchar(')');
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_WM_MODULE_NAME, 0, &instance->config.wm, FF_WM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmProcessName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmPrettyName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmProtocolName}
        });
    }
}
