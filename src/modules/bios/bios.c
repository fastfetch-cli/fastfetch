#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bios/bios.h"
#include "modules/bios/bios.h"
#include "util/stringUtils.h"

#define FF_BIOS_NUM_FORMAT_ARGS 4

void ffPrintBios(FFBiosOptions* options)
{
    FFBiosResult bios;
    ffStrbufInit(&bios.date);
    ffStrbufInit(&bios.release);
    ffStrbufInit(&bios.vendor);
    ffStrbufInit(&bios.version);

    const char* error = ffDetectBios(&bios);

    if(error)
    {
        ffPrintError(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        goto exit;
    }

    if(bios.version.length == 0)
    {
        ffPrintError(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, "bios_version is not set.");
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&bios.version, stdout);
        if (bios.release.length)
            printf(" (%s)", bios.release.chars);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, FF_BIOS_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &bios.date},
            {FF_FORMAT_ARG_TYPE_STRBUF, &bios.release},
            {FF_FORMAT_ARG_TYPE_STRBUF, &bios.vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &bios.version},
        });
    }

exit:
    ffStrbufDestroy(&bios.date);
    ffStrbufDestroy(&bios.release);
    ffStrbufDestroy(&bios.vendor);
    ffStrbufDestroy(&bios.version);
}

bool ffParseBiosCommandOptions(FFBiosOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BIOS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseBiosJsonObject(FFBiosOptions* options, yyjson_val* module)
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

        ffPrintError(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateBiosJsonConfig(FFBiosOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyBiosOptions))) FFBiosOptions defaultOptions;
    ffInitBiosOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateBiosJsonResult(FF_MAYBE_UNUSED FFBiosOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFBiosResult bios;
    ffStrbufInit(&bios.date);
    ffStrbufInit(&bios.release);
    ffStrbufInit(&bios.vendor);
    ffStrbufInit(&bios.version);

    const char* error = ffDetectBios(&bios);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    if (bios.version.length == 0)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "bios_version is not set.");
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "date", &bios.date);
    yyjson_mut_obj_add_strbuf(doc, obj, "release", &bios.release);
    yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &bios.vendor);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &bios.version);

exit:
    ffStrbufDestroy(&bios.date);
    ffStrbufDestroy(&bios.release);
    ffStrbufDestroy(&bios.vendor);
    ffStrbufDestroy(&bios.version);
}

void ffPrintBiosHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_BIOS_MODULE_NAME, "{4} ({2})", FF_BIOS_NUM_FORMAT_ARGS, (const char* []) {
        "bios date",
        "bios release",
        "bios vendor",
        "bios version"
    });
}

void ffInitBiosOptions(FFBiosOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BIOS_MODULE_NAME,
        ffParseBiosCommandOptions,
        ffParseBiosJsonObject,
        ffPrintBios,
        ffGenerateBiosJsonResult,
        ffPrintBiosHelpFormat,
        ffGenerateBiosJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyBiosOptions(FFBiosOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
