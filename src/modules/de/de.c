#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "detection/de/de.h"
#include "modules/de/de.h"
#include "util/stringUtils.h"

#define FF_DE_NUM_FORMAT_ARGS 3

void ffPrintDE(FFDEOptions* options)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->dePrettyName.length == 0)
    {
        ffPrintError(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No DE found");
        return;
    }

    FF_STRBUF_AUTO_DESTROY version = ffStrbufCreate();
    ffDetectDEVersion(&result->dePrettyName, &version, options);

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        ffStrbufWriteTo(&result->dePrettyName, stdout);

        if(version.length > 0)
        {
            putchar(' ');
            ffStrbufWriteTo(&version, stdout);
        }

        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_DE_NUM_FORMAT_ARGS, ((FFformatarg[]){
            FF_FORMAT_ARG(result->deProcessName, "process-name"),
            FF_FORMAT_ARG(result->dePrettyName, "pretty-name"),
            FF_FORMAT_ARG(version, "version")
        }));
    }
}

bool ffParseDECommandOptions(FFDEOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_DE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "slow-version-detection"))
    {
        options->slowVersionDetection = ffOptionParseBoolean(value);
        return true;
    }

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

        if (ffStrEqualsIgnCase(key, "slowVersionDetection"))
        {
            options->slowVersionDetection = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateDEJsonConfig(FFDEOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyDEOptions))) FFDEOptions defaultOptions;
    ffInitDEOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (defaultOptions.slowVersionDetection != options->slowVersionDetection)
        yyjson_mut_obj_add_bool(doc, module, "slowVersionDetection", options->slowVersionDetection);
}

void ffGenerateDEJsonResult(FF_MAYBE_UNUSED FFDEOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->dePrettyName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No DE found");
        return;
    }

    FF_STRBUF_AUTO_DESTROY version = ffStrbufCreate();
    ffDetectDEVersion(&result->dePrettyName, &version, options);

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->deProcessName);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->dePrettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &version);
}

void ffPrintDEHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_DE_MODULE_NAME, "{2} {3}", FF_DE_NUM_FORMAT_ARGS, ((const char* []) {
        "DE process name - process-name",
        "DE pretty name - pretty-name",
        "DE version - version"
    }));
}

void ffInitDEOptions(FFDEOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_DE_MODULE_NAME,
        "Print desktop environment name",
        ffParseDECommandOptions,
        ffParseDEJsonObject,
        ffPrintDE,
        ffGenerateDEJsonResult,
        ffPrintDEHelpFormat,
        ffGenerateDEJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->slowVersionDetection = false;
}

void ffDestroyDEOptions(FFDEOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
