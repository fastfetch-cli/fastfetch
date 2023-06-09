#include "fastfetch.h"
#include "common/printing.h"
#include "detection/chassis/chassis.h"
#include "modules/chassis/chassis.h"

#define FF_CHASSIS_NUM_FORMAT_ARGS 3

void ffPrintChassis(FFinstance* instance, FFChassisOptions* options)
{
    FFChassisResult result;
    ffDetectChassis(&result);

    if(result.error.length > 0)
    {
        ffPrintError(instance, FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, "%*s", result.error.length, result.error.chars);
        goto exit;
    }

    if(result.chassisType.length == 0)
    {
        ffPrintError(instance, FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs, "chassis_type is not set by O.E.M.");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CHASSIS_MODULE_NAME, 0, &options->moduleArgs.key);

        FF_STRBUF_AUTO_DESTROY output = ffStrbufCreateCopy(&result.chassisType);

        if(result.chassisVersion.length > 0)
            ffStrbufAppendF(&output, " (%s)", result.chassisVersion.chars);

        ffStrbufPutTo(&output, stdout);
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
    ffStrbufDestroy(&result.error);
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

#ifdef FF_HAVE_JSONC
void ffParseChassisJsonObject(FFinstance* instance, json_object* module)
{
    FFChassisOptions __attribute__((__cleanup__(ffDestroyChassisOptions))) options;
    ffInitChassisOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_CHASSIS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintChassis(instance, &options);
}
#endif
