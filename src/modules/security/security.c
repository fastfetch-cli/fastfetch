#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/security/security.h"
#include "modules/security/security.h"
#include "util/stringUtils.h"

bool ffPrintSecurity(FFSecurityOptions* options)
{
    bool success = false;
    FFSecurityResult result;
    ffStrbufInit(&result.tpmStatus);
    ffStrbufInit(&result.secureBootStatus);

    const char* error = ffDetectSecurity(&result);
    if(error)
    {
        ffPrintError(FF_SECURITY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_SECURITY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        
        bool first = true;
        if (result.tpmStatus.length > 0 && !ffStrbufEqualS(&result.tpmStatus, "TPM Not Available"))
        {
            ffStrbufWriteTo(&result.tpmStatus, stdout);
            first = false;
        }
        
        if (result.secureBootStatus.length > 0)
        {
            if (!first)
                printf(", ");
            ffStrbufWriteTo(&result.secureBootStatus, stdout);
        }
        
        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_SECURITY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result.tpmStatus, "tpmStatus"),
            FF_FORMAT_ARG(result.secureBootStatus, "secureBootStatus"),
        }));
    }
    success = true;

exit:
    ffStrbufDestroy(&result.tpmStatus);
    ffStrbufDestroy(&result.secureBootStatus);

    return success;
}

void ffParseSecurityJsonObject(FFSecurityOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_SECURITY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateSecurityJsonConfig(FFSecurityOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateSecurityJsonResult(FF_MAYBE_UNUSED FFSecurityOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFSecurityResult result;
    ffStrbufInit(&result.tpmStatus);
    ffStrbufInit(&result.secureBootStatus);

    const char* error = ffDetectSecurity(&result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "tpmStatus", &result.tpmStatus);
    yyjson_mut_obj_add_strbuf(doc, obj, "secureBootStatus", &result.secureBootStatus);
    success = true;

exit:
    ffStrbufDestroy(&result.tpmStatus);
    ffStrbufDestroy(&result.secureBootStatus);
    return success;
}

void ffInitSecurityOptions(FFSecurityOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "🔒");
}

void ffDestroySecurityOptions(FFSecurityOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffSecurityModuleInfo = {
    .name = FF_SECURITY_MODULE_NAME,
    .description = "Print security information (TPM, Secure Boot)",
    .initOptions = (void*) ffInitSecurityOptions,
    .destroyOptions = (void*) ffDestroySecurityOptions,
    .parseJsonObject = (void*) ffParseSecurityJsonObject,
    .printModule = (void*) ffPrintSecurity,
    .generateJsonResult = (void*) ffGenerateSecurityJsonResult,
    .generateJsonConfig = (void*) ffGenerateSecurityJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"TPM status", "tpmStatus"},
        {"Secure Boot status", "secureBootStatus"},
    }))
};
