#include "fastfetch.h"
#include "common/printing.h"
#include "detection/chassis/chassis.h"

#define FF_CHASSIS_MODULE_NAME "Chassis"
#define FF_CHASSIS_NUM_FORMAT_ARGS 3

void ffPrintChassis(FFinstance* instance)
{
    FFChassisResult result;
    ffDetectChassis(&result);

    if(result.error.length > 0)
    {
        ffPrintError(instance, FF_CHASSIS_MODULE_NAME, 0, &instance->config.chassis, "%*s", result.error.length, result.error.chars);
        goto exit;
    }

    if(result.chassisType.length == 0)
    {
        ffPrintError(instance, FF_CHASSIS_MODULE_NAME, 0, &instance->config.host, "chassis_type is not set by O.E.M.");
        return;
    }

    if(instance->config.chassis.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CHASSIS_MODULE_NAME, 0, &instance->config.host.key);

        FFstrbuf output;
        ffStrbufInitCopy(&output, &result.chassisType);

        if(result.chassisVersion.length > 0)
            ffStrbufAppendF(&output, " (%s)", &result.chassisVersion.chars);

        ffStrbufPutTo(&output, stdout);

        ffStrbufDestroy(&output);
    }
    else
    {
        ffPrintFormat(instance, FF_CHASSIS_MODULE_NAME, 0, &instance->config.chassis, FF_CHASSIS_NUM_FORMAT_ARGS, (FFformatarg[]) {
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
