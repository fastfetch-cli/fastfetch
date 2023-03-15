#include "fastfetch.h"
#include "common/printing.h"
#include "detection/bios/bios.h"
#include "modules/bios/bios.h"

#define FF_BIOS_MODULE_NAME "Bios"
#define FF_BIOS_NUM_FORMAT_ARGS 4

void ffPrintBios(FFinstance* instance, FFBiosOptions* options)
{
    FFBiosResult result;
    ffDetectBios(&result);

    if(result.error.length > 0)
    {
        ffPrintError(instance, FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, "%*s", result.error.length, result.error.chars);
        goto exit;
    }

    if(result.biosRelease.length == 0)
    {
        ffPrintError(instance, FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, "bios_release is not set.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BIOS_MODULE_NAME, 0, &options->moduleArgs.key);
        puts(result.biosRelease.chars);
    }
    else
    {
        ffPrintFormat(instance, FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, FF_BIOS_NUM_FORMAT_ARGS, (FFformatarg[]) {
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

void ffInitBiosOptions(FFBiosOptions* options)
{
    options->moduleName = FF_BIOS_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseBiosCommandOptions(FFBiosOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BIOS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyBiosOptions(FFBiosOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

#ifdef FF_HAVE_JSONC
bool ffParseBiosJsonObject(FFinstance* instance, const char* type, JSONCData* data, json_object* module)
{
    if (strcasecmp(type, FF_BIOS_MODULE_NAME) != 0)
        return false;

    FFBiosOptions __attribute__((__cleanup__(ffDestroyBiosOptions))) options;
    ffInitBiosOptions(&options);

    FF_JSON_OBJECT_OBJECT_FOREACH(data, module, key, val)
    {
        if (strcasecmp(key, "type") == 0)
            continue;

        if (ffJsonConfigParseModuleArgs(data, key, val, &options.moduleArgs))
            continue;

        ffPrintError(instance, FF_BIOS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
    }

    ffPrintBios(instance, &options);
    return true;
}
#endif
