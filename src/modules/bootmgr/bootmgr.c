#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/bootmgr/bootmgr.h"
#include "modules/bootmgr/bootmgr.h"
#include "util/stringUtils.h"

bool ffPrintBootmgr(FFBootmgrOptions* options)
{
    bool success = false;
    FFBootmgrResult bootmgr = {
        .name = ffStrbufCreate(),
        .firmware = ffStrbufCreate(),
    };

    const char* error = ffDetectBootmgr(&bootmgr);

    if(error)
    {
        ffPrintError(FF_BOOTMGR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }
    else
    {
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
            FF_PRINT_FORMAT_CHECKED(FF_BOOTMGR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                FF_FORMAT_ARG(bootmgr.name, "name"),
                FF_FORMAT_ARG(bootmgr.firmware, "firmware-path"),
                FF_FORMAT_ARG(firmwareName, "firmware-name"),
                FF_FORMAT_ARG(bootmgr.secureBoot, "secure-boot"),
                FF_FORMAT_ARG(bootmgr.order, "order"),
            }));
        }
    }
    success = true;

exit:
    ffStrbufDestroy(&bootmgr.name);
    ffStrbufDestroy(&bootmgr.firmware);

    return success;
}

void ffParseBootmgrJsonObject(FFBootmgrOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_BOOTMGR_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateBootmgrJsonConfig(FFBootmgrOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateBootmgrJsonResult(FF_MAYBE_UNUSED FFBootmgrOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
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
    yyjson_mut_obj_add_uint(doc, obj, "order", bootmgr.order);
    yyjson_mut_obj_add_bool(doc, obj, "secureBoot", bootmgr.secureBoot);
    success = true;

exit:
    ffStrbufDestroy(&bootmgr.name);
    ffStrbufDestroy(&bootmgr.firmware);
    return success;
}

void ffInitBootmgrOptions(FFBootmgrOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyBootmgrOptions(FFBootmgrOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffBootmgrModuleInfo = {
    .name = FF_BOOTMGR_MODULE_NAME,
    .description = "Print information of 2nd-stage bootloader (name, firmware, etc)",
    .initOptions = (void*) ffInitBootmgrOptions,
    .destroyOptions = (void*) ffDestroyBootmgrOptions,
    .parseJsonObject = (void*) ffParseBootmgrJsonObject,
    .printModule = (void*) ffPrintBootmgr,
    .generateJsonResult = (void*) ffGenerateBootmgrJsonResult,
    .generateJsonConfig = (void*) ffGenerateBootmgrJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Name / description", "name"},
        {"Firmware file path", "firmware-path"},
        {"Firmware file name", "firmware-name"},
        {"Is secure boot enabled", "secure-boot"},
        {"Boot order", "order"},
    }))
};
