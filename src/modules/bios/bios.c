#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bios/bios.h"
#include "modules/bios/bios.h"
#include "util/stringUtils.h"

#define FF_BIOS_NUM_FORMAT_ARGS 4

void ffPrintBios(FFinstance* instance, FFBiosOptions* options)
{
    FFBiosResult bios;
    ffStrbufInit(&bios.biosDate);
    ffStrbufInit(&bios.biosRelease);
    ffStrbufInit(&bios.biosVendor);
    ffStrbufInit(&bios.biosVersion);

    const char* error = ffDetectBios(&bios);

    if(error)
    {
        ffPrintError(instance, FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        goto exit;
    }

    if(bios.biosRelease.length == 0)
    {
        ffPrintError(instance, FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, "bios_release is not set.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BIOS_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);
        ffStrbufWriteTo(&bios.biosRelease, stdout);
        if (bios.biosVersion.length)
            printf(" (%s)", bios.biosVersion.chars);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, FF_BIOS_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &bios.biosDate},
            {FF_FORMAT_ARG_TYPE_STRBUF, &bios.biosRelease},
            {FF_FORMAT_ARG_TYPE_STRBUF, &bios.biosVendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &bios.biosVersion},
        });
    }

exit:
    ffStrbufDestroy(&bios.biosDate);
    ffStrbufDestroy(&bios.biosRelease);
    ffStrbufDestroy(&bios.biosVendor);
    ffStrbufDestroy(&bios.biosVersion);
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

void ffParseBiosJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFBiosOptions __attribute__((__cleanup__(ffDestroyBiosOptions))) options;
    ffInitBiosOptions(&options);

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

            ffPrintError(instance, FF_BIOS_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintBios(instance, &options);
}
