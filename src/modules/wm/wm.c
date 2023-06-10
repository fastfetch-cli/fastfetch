#include "fastfetch.h"
#include "common/printing.h"
#include "detection/displayserver/displayserver.h"
#include "modules/wm/wm.h"

#define FF_WM_NUM_FORMAT_ARGS 3

void ffPrintWM(FFinstance* instance, FFWMOptions* options)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer(instance);

    if(result->wmPrettyName.length == 0)
    {
        ffPrintError(instance, FF_WM_MODULE_NAME, 0, &options->moduleArgs, "No WM found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WM_MODULE_NAME, 0, &options->moduleArgs.key);

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
        ffPrintFormat(instance, FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_WM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmProcessName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmPrettyName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmProtocolName}
        });
    }
}

void ffInitWMOptions(FFWMOptions* options)
{
    options->moduleName = FF_WM_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseWMCommandOptions(FFWMOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WM_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyWMOptions(FFWMOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
void ffParseWMJsonObject(FFinstance* instance, json_object* module)
{
    FFWMOptions __attribute__((__cleanup__(ffDestroyWMOptions))) options;
    ffInitWMOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_WM_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintWM(instance, &options);
}
#endif
