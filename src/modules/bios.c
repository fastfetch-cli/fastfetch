#include "fastfetch.h"
#include "common/printing.h"
#include "detection/bios/bios.h"

#define FF_BIOS_MODULE_NAME "Bios"
#define FF_BIOS_NUM_FORMAT_ARGS 4

void ffPrintBios(FFinstance* instance)
{
    FFBiosResult result;
    ffDetectBios(&result);

    if(result.error.length > 0)
    {
        ffPrintError(instance, FF_BIOS_MODULE_NAME, 0, &instance->config.bios, "%*s", result.error.length, result.error.chars);
        goto exit;
    }

    if(result.biosRelease.length == 0)
    {
        ffPrintError(instance, FF_BIOS_MODULE_NAME, 0, &instance->config.bios, "bios_release is not set.");
        goto exit;
    }

    if(instance->config.bios.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BIOS_MODULE_NAME, 0, &instance->config.bios.key);
        puts(result.biosRelease.chars);
    }
    else
    {
        ffPrintFormat(instance, FF_BIOS_MODULE_NAME, 0, &instance->config.bios, FF_BIOS_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.biosDate},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.biosRelease},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.biosVendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.biosVersion},
        });
    }

exit:
    ffStrbufDestroy(&result.biosDate);
    ffStrbufDestroy(&result.biosRelease);
    ffStrbufDestroy(&result.biosVendor);
    ffStrbufDestroy(&result.biosVersion);
    ffStrbufDestroy(&result.error);
}
