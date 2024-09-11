#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bootmgr/bootmgr.h"
#include "modules/bootmgr/bootmgr.h"
#include "util/stringUtils.h"

#define FF_BOOTMGR_NUM_FORMAT_ARGS 4

void ffPrintBootmgr(FFBootmgrOptions* options)
{
    FFBootmgrResult bootmgr = {
        .name = ffStrbufCreate(),
        .firmware = ffStrbufCreate(),
    };

    const char* error = ffDetectBootmgr(&bootmgr);

    if(error)
    {
        ffPrintError(FF_BOOTMGR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    FF_STRBUF_AUTO_DESTROY firmwareName = ffStrbufCreateCopy(&bootmgr.firmware);
    #ifndef __APPLE__
    ffStrbufSubstrAfterLastC(&firmwareName, '\\');
    #else
    ffStrbufSubstrAfterLastC(&firmwareName, '/');
    #endif

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_BOOTMGR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&bootmgr.name, stdout);
        if (firmwareName.length > 0)
            printf(" - %s\n", firmwareName.chars);
        else
            putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_BOOTMGR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_BOOTMGR_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(bootmgr.name, "name"),
            FF_FORMAT_ARG(bootmgr.firmware, "firmware-path"),
            FF_FORMAT_ARG(firmwareName, "firmware-name"),
            FF_FORMAT_ARG(bootmgr.secureBoot, "secure-boot"),
        }));
    }

    ffStrbufDestroy(&bootmgr.name);
    ffStrbufDestroy(&bootmgr.firmware);
}

bool ffParseBootmgrCommandOptions(FFBootmgrOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BOOTMGR_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffParseBootmgrJsonObject(FFBootmgrOptions* options, yyjson_val* module)
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

        ffPrintError(FF_BOOTMGR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateBootmgrJsonConfig(FFBootmgrOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyBootmgrOptions))) FFBootmgrOptions defaultOptions;
    ffInitBootmgrOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGenerateBootmgrJsonResult(FF_MAYBE_UNUSED FFBootmgrOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFBootmgrResult bootmgr = {
        .name = ffStrbufCreate(),
        .firmware = ffStrbufCreate(),
    };

    const char* error = ffDetectBootmgr(&bootmgr);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &bootmgr.name);
    yyjson_mut_obj_add_strbuf(doc, obj, "firmware", &bootmgr.firmware);
    yyjson_mut_obj_add_bool(doc, obj, "secureBoot", bootmgr.secureBoot);

exit:
    ffStrbufDestroy(&bootmgr.name);
    ffStrbufDestroy(&bootmgr.firmware);
}

void ffPrintBootmgrHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_BOOTMGR_MODULE_NAME, "{4} ({2})", FF_BOOTMGR_NUM_FORMAT_ARGS, ((const char* []) {
        "Name / description - name",
        "Firmware file path - firmware-path",
        "Firmware file name - firmware-name",
        "Is secure boot enabled - secure-boot",
    }));
}

void ffInitBootmgrOptions(FFBootmgrOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BOOTMGR_MODULE_NAME,
        "Print information of 2nd-stage bootloader (name, firmware, etc)",
        ffParseBootmgrCommandOptions,
        ffParseBootmgrJsonObject,
        ffPrintBootmgr,
        ffGenerateBootmgrJsonResult,
        ffPrintBootmgrHelpFormat,
        ffGenerateBootmgrJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyBootmgrOptions(FFBootmgrOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
