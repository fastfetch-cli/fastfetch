#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bios/bios.h"
#include "modules/bios/bios.h"
#include "util/stringUtils.h"

#define FF_BIOS_NUM_FORMAT_ARGS 5

void ffPrintBios(FFBiosOptions* options)
{
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
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, 2, ((FFformatarg[]){
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
        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_BIOS_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(bios.date, "date"),
            FF_FORMAT_ARG(bios.release, "release"),
            FF_FORMAT_ARG(bios.vendor, "vendor"),
            FF_FORMAT_ARG(bios.version, "version"),
            FF_FORMAT_ARG(bios.type, "type"),
        }));
    }

exit:
    ffStrbufDestroy(&bios.date);
    ffStrbufDestroy(&bios.release);
    ffStrbufDestroy(&bios.vendor);
    ffStrbufDestroy(&bios.version);
    ffStrbufDestroy(&bios.type);
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

        ffPrintError(FF_BIOS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
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

exit:
    ffStrbufDestroy(&bios.date);
    ffStrbufDestroy(&bios.release);
    ffStrbufDestroy(&bios.vendor);
    ffStrbufDestroy(&bios.version);
    ffStrbufDestroy(&bios.type);
}

void ffPrintBiosHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_BIOS_MODULE_NAME, "{4} ({2})", FF_BIOS_NUM_FORMAT_ARGS, ((const char* []) {
        "bios date - date",
        "bios release - release",
        "bios vendor - vendor",
        "bios version - version",
        "firmware type - type",
    }));
}

void ffInitBiosOptions(FFBiosOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BIOS_MODULE_NAME,
        "Print information of 1st-stage bootloader (name, version, release date, etc)",
        ffParseBiosCommandOptions,
        ffParseBiosJsonObject,
        ffPrintBios,
        ffGenerateBiosJsonResult,
        ffPrintBiosHelpFormat,
        ffGenerateBiosJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyBiosOptions(FFBiosOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
