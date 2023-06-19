#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/chassis/chassis.h"
#include "modules/chassis/chassis.h"
#include "util/stringUtils.h"

#define FF_CHASSIS_NUM_FORMAT_ARGS 3

void ffPrintChassis(FFinstance* instance, FFChassisOptions* options)
{
    FFChassisResult result;
    ffStrbufInit(&result.chassisType);
    ffStrbufInit(&result.chassisVendor);
    ffStrbufInit(&result.chassisVersion);

    const char* error = ffDetectChassis(&result);

    if(error)
    {
        ffPrintError(instance, FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        goto exit;
    }

    if(result.chassisType.length == 0)
    {
        ffPrintError(instance, FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, "chassis_type is not set by O.E.M.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        ffStrbufWriteTo(&result.chassisType, stdout);
        if (result.chassisVersion.length)
            printf(" (%s)", result.chassisVersion.chars);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, FF_CHASSIS_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.chassisType},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.chassisVendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.chassisVersion},
        });
    }

exit:
    ffStrbufDestroy(&result.chassisType);
    ffStrbufDestroy(&result.chassisVendor);
    ffStrbufDestroy(&result.chassisVersion);
}

void ffInitChassisOptions(FFChassisOptions* options)
{
    options->moduleName = FF_CHASSIS_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseChassisCommandOptions(FFChassisOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CHASSIS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyChassisOptions(FFChassisOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseChassisJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFChassisOptions __attribute__((__cleanup__(ffDestroyChassisOptions))) options;
    ffInitChassisOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_CHASSIS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintChassis(instance, &options);
}
