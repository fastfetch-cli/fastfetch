#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "modules/de/de.h"
#include "util/stringUtils.h"

#define FF_DE_NUM_FORMAT_ARGS 3

void ffPrintDE(FFDEOptions* options)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->dePrettyName.length == 0)
    {
        ffPrintError(FF_DE_MODULE_NAME, 0, &options->moduleArgs, "No DE found");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        ffStrbufWriteTo(&result->dePrettyName, stdout);

        if(result->deVersion.length > 0)
        {
            putchar(' ');
            ffStrbufWriteTo(&result->deVersion, stdout);
        }

        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_DE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->deProcessName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->dePrettyName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->deVersion}
        });
    }
}

bool ffParseDECommandOptions(FFDEOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseDEJsonObject(FFDEOptions* options, yyjson_val* module)
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

        ffPrintError(FF_DE_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateDEJsonResult(FF_MAYBE_UNUSED FFDEOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->dePrettyName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No DE found");
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->deProcessName);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->dePrettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result->deVersion);
}

void ffPrintDEHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_DE_MODULE_NAME, "{2} {3}", FF_DE_NUM_FORMAT_ARGS, (const char* []) {
        "DE process name",
        "DE pretty name",
        "DE version"
    });
}

void ffInitDEOptions(FFDEOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_DE_MODULE_NAME, ffParseDECommandOptions, ffParseDEJsonObject, ffPrintDE, ffGenerateDEJsonResult, ffPrintDEHelpFormat);
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyDEOptions(FFDEOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
