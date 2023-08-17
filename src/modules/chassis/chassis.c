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

    const char* error = ffDetectChassis(&result);

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

void ffInitChassisOptions(FFChassisOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_CHASSIS_MODULE_NAME, ffParseChassisCommandOptions, ffParseChassisJsonObject, ffPrintChassis);
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

void ffParseChassisJsonObject(FFChassisOptions* options, yyjson_val* module)
{
    if (module)
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

            ffPrintError(FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
        }
    }
}
