#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/chassis/chassis.h"
#include "modules/chassis/chassis.h"
#include "util/stringUtils.h"

void ffPrintChassis(FFChassisOptions* options)
{
    FFChassisResult result;
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.serial);

    const char* error = ffDetectChassis(&result);

    if(error)
    {
        ffPrintError(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(result.type.length == 0)
    {
        ffPrintError(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "chassis_type is not set by O.E.M.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result.type, stdout);
        if (result.version.length)
            printf(" (%s)", result.version.chars);
        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result.type, "type"),
            FF_FORMAT_ARG(result.vendor, "vendor"),
            FF_FORMAT_ARG(result.version, "version"),
            FF_FORMAT_ARG(result.serial, "serial"),
        }));
    }

exit:
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.serial);
}

bool ffParseChassisCommandOptions(FFChassisOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CHASSIS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseChassisJsonObject(FFChassisOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateChassisJsonConfig(FFChassisOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyChassisOptions))) FFChassisOptions defaultOptions;
    ffInitChassisOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateChassisJsonResult(FF_MAYBE_UNUSED FFChassisOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFChassisResult result;
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.serial);

    const char* error = ffDetectChassis(&result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if(result.type.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "chassis_type is not set by O.E.M.");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "type", &result.type);
    yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &result.vendor);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);
    yyjson_mut_obj_add_strbuf(doc, obj, "serial", &result.serial);

exit:
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.serial);
}

static FFModuleBaseInfo ffModuleInfo = {
    .name = FF_CHASSIS_MODULE_NAME,
    .description = "Print chassis type (desktop, laptop, etc)",
    .parseCommandOptions = (void*) ffParseChassisCommandOptions,
    .parseJsonObject = (void*) ffParseChassisJsonObject,
    .printModule = (void*) ffPrintChassis,
    .generateJsonResult = (void*) ffGenerateChassisJsonResult,
    .generateJsonConfig = (void*) ffGenerateChassisJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Chassis type", "type"},
        {"Chassis vendor", "vendor"},
        {"Chassis version", "version"},
        {"Chassis serial number", "serial"},
    })),
};

void ffInitChassisOptions(FFChassisOptions* options)
{
    options->moduleInfo = ffModuleInfo;
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyChassisOptions(FFChassisOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
