#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/displayserver/displayserver.h"
#include "detection/de/de.h"
#include "modules/de/de.h"
#include "util/stringUtils.h"

bool ffPrintDE(FFDEOptions* options)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->dePrettyName.length == 0)
    {
        ffPrintError(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No DE found");
        return false;
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
        FF_PRINT_FORMAT_CHECKED(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result->deProcessName, "process-name"),
            FF_FORMAT_ARG(result->dePrettyName, "pretty-name"),
            FF_FORMAT_ARG(version, "version")
        }));
    }

    return true;
}

void ffParseDEJsonObject(FFDEOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "slowVersionDetection"))
        {
            options->slowVersionDetection = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_DE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateDEJsonConfig(FFDEOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "slowVersionDetection", options->slowVersionDetection);
}

bool ffGenerateDEJsonResult(FF_MAYBE_UNUSED FFDEOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFDisplayServerResult* result = ffConnectDisplayServer();

    if(result->dePrettyName.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No DE found");
        return false;
    }

    FF_STRBUF_AUTO_DESTROY version = ffStrbufCreate();
    ffDetectDEVersion(&result->dePrettyName, &version, options);

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "processName", &result->deProcessName);
    yyjson_mut_obj_add_strbuf(doc, obj, "prettyName", &result->dePrettyName);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &version);
    return true;
}

void ffInitDEOptions(FFDEOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->slowVersionDetection = false;
}

void ffDestroyDEOptions(FFDEOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffDEModuleInfo = {
    .name = FF_DE_MODULE_NAME,
    .description = "Print desktop environment name",
    .initOptions = (void*) ffInitDEOptions,
    .destroyOptions = (void*) ffDestroyDEOptions,
    .parseJsonObject = (void*) ffParseDEJsonObject,
    .printModule = (void*) ffPrintDE,
    .generateJsonResult = (void*) ffGenerateDEJsonResult,
    .generateJsonConfig = (void*) ffGenerateDEJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"DE process name", "process-name"},
        {"DE pretty name", "pretty-name"},
        {"DE version", "version"},
    }))
};
