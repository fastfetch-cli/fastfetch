#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/tpm/tpm.h"
#include "modules/tpm/tpm.h"
#include "util/stringUtils.h"

void ffPrintTPM(FFTPMOptions* options)
{
    FFTPMResult result = {
        .version = ffStrbufCreate(),
        .description = ffStrbufCreate()
    };
    const char* error = ffDetectTPM(&result);

    if(error)
    {
        ffPrintError(FF_TPM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_TPM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        if (result.description.length > 0)
            ffStrbufPutTo(&result.description, stdout);
        else
            ffStrbufPutTo(&result.version, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_TPM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result.version, "version"),
            FF_FORMAT_ARG(result.description, "description"),
        }));
    }

    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.description);
}

bool ffParseTPMCommandOptions(FFTPMOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_TPM_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseTPMJsonObject(FFTPMOptions* options, yyjson_val* module)
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

        ffPrintError(FF_TPM_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateTPMJsonConfig(FFTPMOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyTPMOptions))) FFTPMOptions defaultOptions;
    ffInitTPMOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateTPMJsonResult(FF_MAYBE_UNUSED FFTerminalOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFTPMResult result = {
        .version = ffStrbufCreate(),
        .description = ffStrbufCreate()
    };
    const char* error = ffDetectTPM(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);
    yyjson_mut_obj_add_strbuf(doc, obj, "description", &result.description);

    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.description);
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_TPM_MODULE_NAME,
    .description = "Print info of Trusted Platform Module (TPM) Security Device",
    .parseCommandOptions = (void*) ffParseTPMCommandOptions,
    .parseJsonObject = (void*) ffParseTPMJsonObject,
    .printModule = (void*) ffPrintTPM,
    .generateJsonResult = (void*) ffGenerateTPMJsonResult,
    .generateJsonConfig = (void*) ffGenerateTPMJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"TPM device version", "version"},
        {"TPM general description", "description"},
    }))
};

void ffInitTPMOptions(FFTPMOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyTPMOptions(FFTPMOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
