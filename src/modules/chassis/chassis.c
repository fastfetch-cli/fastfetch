#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/chassis/chassis.h"
#include "modules/chassis/chassis.h"
#include "util/stringUtils.h"

#define FF_CHASSIS_NUM_FORMAT_ARGS 3

void ffPrintChassis(FFChassisOptions* options)
{
    FFChassisResult result;
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.version);

    const char* error = ffDetectChassis(&result, options);

    if(error)
    {
        ffPrintError(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        goto exit;
    }

    if(result.type.length == 0)
    {
        ffPrintError(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, "chassis_type is not set by O.E.M.");
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
        ffPrintFormat(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, FF_CHASSIS_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.type},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.version},
        });
    }

exit:
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.version);
}

bool ffParseChassisCommandOptions(FFChassisOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CHASSIS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    #ifdef _WIN32
    if (ffStrEqualsIgnCase(subKey, "use-wmi"))
    {
        options->useWmi = ffOptionParseBoolean(value);
        return true;
    }
    #endif

    return false;
}

void ffParseChassisJsonObject(FFChassisOptions* options, yyjson_val* module)
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

        #ifdef _WIN32
        if (ffStrEqualsIgnCase(key, "useWmi"))
        {
            options->useWmi = yyjson_get_bool(val);
            continue;
        }
        #endif

        ffPrintError(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateChassisJsonConfig(FFChassisOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyChassisOptions))) FFChassisOptions defaultOptions;
    ffInitChassisOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    #ifdef _WIN32
    if (options->useWmi != defaultOptions.useWmi)
        yyjson_mut_obj_add_bool(doc, module, "useWmi", options->useWmi);
    #endif
}

void ffGenerateChassisJsonResult(FF_MAYBE_UNUSED FFChassisOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFChassisResult result;
    ffStrbufInit(&result.type);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.version);

    const char* error = ffDetectChassis(&result, options);

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

exit:
    ffStrbufDestroy(&result.type);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.version);
}

void ffPrintChassisHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_CHASSIS_MODULE_NAME, "{1}", FF_CHASSIS_NUM_FORMAT_ARGS, (const char* []) {
        "chassis type",
        "chassis vendor",
        "chassis version"
    });
}

void ffInitChassisOptions(FFChassisOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_CHASSIS_MODULE_NAME,
        ffParseChassisCommandOptions,
        ffParseChassisJsonObject,
        ffPrintChassis,
        ffGenerateChassisJsonResult,
        ffPrintChassisHelpFormat,
        ffGenerateChassisJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    #ifdef _WIN32
    options->useWmi = false;
    #endif
}

void ffDestroyChassisOptions(FFChassisOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
