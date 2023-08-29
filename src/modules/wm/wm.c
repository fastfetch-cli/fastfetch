#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "modules/wm/wm.h"
#include "util/stringUtils.h"

#define FF_WM_NUM_FORMAT_ARGS 3

void ffPrintWM(FFWMOptions* options)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->wmPrettyName.length == 0)
    {
        ffPrintError(FF_WM_MODULE_NAME, 0, &options->moduleArgs, "No WM found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

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
        ffPrintFormat(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_WM_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmProcessName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmPrettyName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->wmProtocolName}
        });
    }
}

void ffInitWMOptions(FFWMOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_WM_MODULE_NAME, ffParseWMCommandOptions, ffParseWMJsonObject, ffPrintWM, NULL);
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

void ffParseWMJsonObject(FFWMOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_WM_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
