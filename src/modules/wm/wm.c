#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "detection/wm/wm.h"
#include "modules/wm/wm.h"
#include "util/stringUtils.h"

#define FF_WM_NUM_FORMAT_ARGS 4

void ffPrintWM(FFWMOptions* options)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->wmPrettyName.length == 0)
    {
        ffPrintError(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No WM found");
        return;
    }

    FF_STRBUF_AUTO_DESTROY pluginName = ffStrbufCreate();
    if(options->detectPlugin)
        ffDetectWMPlugin(&pluginName);

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

        if(pluginName.length > 0)
        {
            fputs(" (with ", stdout);
            ffStrbufWriteTo(&pluginName, stdout);
            putchar(')');
        }

        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_WM_NUM_FORMAT_ARGS, ((FFformatarg[]){
            FF_FORMAT_ARG(result->wmProcessName, "process-name"),
            FF_FORMAT_ARG(result->wmPrettyName, "pretty-name"),
            FF_FORMAT_ARG(result->wmProtocolName, "protocol-name"),
            FF_FORMAT_ARG(pluginName, "plugin-name"),
        }));
    }
}

bool ffParseWMCommandOptions(FFWMOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_WM_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "detect-plugin"))
    {
        options->detectPlugin = ffOptionParseBoolean(value);
        return true;
    }

    return false;
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

        if (ffStrEqualsIgnCase(key, "detectPlugin"))
        {
            options->detectPlugin = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateWMJsonConfig(FFWMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyWMOptions))) FFWMOptions defaultOptions;
    ffInitWMOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (options->detectPlugin != defaultOptions.detectPlugin)
        yyjson_mut_obj_add_bool(doc, module, "detectPlugin", options->detectPlugin);
}

void ffGenerateWMJsonResult(FF_MAYBE_UNUSED FFWMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->wmPrettyName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No WM found");
        return;
    }

    FF_STRBUF_AUTO_DESTROY pluginName = ffStrbufCreate();
    if(options->detectPlugin)
        ffDetectWMPlugin(&pluginName);

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->wmProcessName);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->wmPrettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "protocolName", &result->wmProtocolName);
    yyjson_mut_obj_add_strbuf(doc, obj, "pluginName", &pluginName);
}

void ffPrintWMHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_WM_MODULE_NAME, "{2} ({3})", FF_WM_NUM_FORMAT_ARGS, ((const char* []) {
        "WM process name - process-name",
        "WM pretty name - pretty-name",
        "WM protocol name - protocol-name",
        "WM plugin name - plugin-name",
    }));
}

void ffInitWMOptions(FFWMOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_WM_MODULE_NAME,
        "Print window manager name and version",
        ffParseWMCommandOptions,
        ffParseWMJsonObject,
        ffPrintWM,
        ffGenerateWMJsonResult,
        ffPrintWMHelpFormat,
        ffGenerateWMJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");
    options->detectPlugin = false;
}

void ffDestroyWMOptions(FFWMOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
