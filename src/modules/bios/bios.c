#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bios/bios.h"
#include "modules/bios/bios.h"
#include "util/stringUtils.h"

bool ffPrintBios(FFBiosOptions* options)
{
    bool success = false;
    FFBiosResult bios;
    ffStrbufInit(&bios.date);
    ffStrbufInit(&bios.release);
    ffStrbufInit(&bios.vendor);
    ffStrbufInit(&bios.version);
    ffStrbufInit(&bios.type);

    const char* error = ffDetectBios(&bios);

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    if(error)
    {
        ffPrintError(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(bios.version.length == 0)
    {
        ffPrintError(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "bios_version is not set.");
        goto exit;
    }

    if(options->moduleArgs.key.length == 0)
    {
        if(bios.type.length == 0)
            ffStrbufSetStatic(&bios.type, "Unknown");
        else if (ffStrbufIgnCaseEqualS(&bios.type, "BIOS"))
            ffStrbufSetStatic(&bios.type, "Legacy");

        ffStrbufSetF(&key, FF_BIOS_MODULE_NAME " (%s)", bios.type.chars);
    }
    else
    {
        ffStrbufClear(&key);
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]) {
            FF_FORMAT_ARG(bios.type, "type"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);
        ffStrbufWriteTo(&bios.version, stdout);
        if (bios.release.length)
            printf(" (%s)\n", bios.release.chars);
        else
            putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
            FF_FORMAT_ARG(bios.date, "date"),
            FF_FORMAT_ARG(bios.release, "release"),
            FF_FORMAT_ARG(bios.vendor, "vendor"),
            FF_FORMAT_ARG(bios.version, "version"),
            FF_FORMAT_ARG(bios.type, "type"),
        }));
    }
    success = true;

exit:
    ffStrbufDestroy(&bios.date);
    ffStrbufDestroy(&bios.release);
    ffStrbufDestroy(&bios.vendor);
    ffStrbufDestroy(&bios.version);
    ffStrbufDestroy(&bios.type);

    return success;
}

void ffParseBiosJsonObject(FFBiosOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateBiosJsonConfig(FFBiosOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateBiosJsonResult(FF_MAYBE_UNUSED FFBiosOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFBiosResult bios;
    ffStrbufInit(&bios.date);
    ffStrbufInit(&bios.release);
    ffStrbufInit(&bios.vendor);
    ffStrbufInit(&bios.version);
    ffStrbufInit(&bios.type);

    const char* error = ffDetectBios(&bios);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "date", &bios.date);
    yyjson_mut_obj_add_strbuf(doc, obj, "release", &bios.release);
    yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &bios.vendor);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &bios.version);
    yyjson_mut_obj_add_strbuf(doc, obj, "type", &bios.type);
    success = true;

exit:
    ffStrbufDestroy(&bios.date);
    ffStrbufDestroy(&bios.release);
    ffStrbufDestroy(&bios.vendor);
    ffStrbufDestroy(&bios.version);
    ffStrbufDestroy(&bios.type);
    return success;
}

void ffInitBiosOptions(FFBiosOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyBiosOptions(FFBiosOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffBiosModuleInfo = {
    .name = FF_BIOS_MODULE_NAME,
    .description = "Print information of 1st-stage bootloader (name, version, release date, etc)",
    .initOptions = (void*) ffInitBiosOptions,
    .destroyOptions = (void*) ffDestroyBiosOptions,
    .parseJsonObject = (void*) ffParseBiosJsonObject,
    .printModule = (void*) ffPrintBios,
    .generateJsonResult = (void*) ffGenerateBiosJsonResult,
    .generateJsonConfig = (void*) ffGenerateBiosJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Bios date", "date"},
        {"Bios release", "release"},
        {"Bios vendor", "vendor"},
        {"Bios version", "version"},
        {"Firmware type", "type"},
    }))
};
