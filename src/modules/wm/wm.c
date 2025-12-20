#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "detection/wm/wm.h"
#include "modules/wm/wm.h"
#include "util/stringUtils.h"

bool ffPrintWM(FFWMOptions* options)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->wmPrettyName.length == 0)
    {
        ffPrintError(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No WM found");
        return false;
    }

    FF_STRBUF_AUTO_DESTROY pluginName = ffStrbufCreate();
    if(options->detectPlugin)
        ffDetectWMPlugin(&pluginName);

    FF_STRBUF_AUTO_DESTROY version = ffStrbufCreate();
    if (instance.config.general.detectVersion)
        ffDetectWMVersion(&result->wmProcessName, &version, options);

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        ffStrbufWriteTo(&result->wmPrettyName, stdout);

        if(version.length > 0)
        {
            putchar(' ');
            ffStrbufWriteTo(&version, stdout);
        }

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
        FF_PRINT_FORMAT_CHECKED(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result->wmProcessName, "process-name"),
            FF_FORMAT_ARG(result->wmPrettyName, "pretty-name"),
            FF_FORMAT_ARG(result->wmProtocolName, "protocol-name"),
            FF_FORMAT_ARG(pluginName, "plugin-name"),
            FF_FORMAT_ARG(version, "version"),
        }));
    }

    return true;
}

void ffParseWMJsonObject(FFWMOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "detectPlugin"))
        {
            options->detectPlugin = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_WM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateWMJsonConfig(FFWMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "detectPlugin", options->detectPlugin);
}

bool ffGenerateWMJsonResult(FF_MAYBE_UNUSED FFWMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->wmPrettyName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No WM found");
        return false;
    }

    FF_STRBUF_AUTO_DESTROY pluginName = ffStrbufCreate();
    if(options->detectPlugin)
        ffDetectWMPlugin(&pluginName);

    FF_STRBUF_AUTO_DESTROY version = ffStrbufCreate();
    if (instance.config.general.detectVersion)
        ffDetectWMVersion(&result->wmProcessName, &version, options);

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->wmProcessName);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->wmPrettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "protocolName", &result->wmProtocolName);
    yyjson_mut_obj_add_strbuf(doc, obj, "pluginName", &pluginName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &version);

    return true;
}

void ffInitWMOptions(FFWMOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
    options->detectPlugin = false;
}

void ffDestroyWMOptions(FFWMOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffWMModuleInfo = {
    .name = FF_WM_MODULE_NAME,
    .description = "Print window manager name and version",
    .initOptions = (void*) ffInitWMOptions,
    .destroyOptions = (void*) ffDestroyWMOptions,
    .parseJsonObject = (void*) ffParseWMJsonObject,
    .printModule = (void*) ffPrintWM,
    .generateJsonResult = (void*) ffGenerateWMJsonResult,
    .generateJsonConfig = (void*) ffGenerateWMJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"WM process name", "process-name"},
        {"WM pretty name", "pretty-name"},
        {"WM protocol name", "protocol-name"},
        {"WM plugin name", "plugin-name"},
        {"WM version", "version"},
    }))
};
