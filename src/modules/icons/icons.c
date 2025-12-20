#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/icons/icons.h"
#include "modules/icons/icons.h"
#include "util/stringUtils.h"

bool ffPrintIcons(FFIconsOptions* options)
{
    bool success = false;
    FFIconsResult result = {
        .icons1 = ffStrbufCreate(),
        .icons2 = ffStrbufCreate(),
    };
    const char* error = ffDetectIcons(&result);

    if(error)
    {
        ffPrintError(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        if (result.icons1.length)
            ffStrbufWriteTo(&result.icons1, stdout);
        if (result.icons2.length)
        {
            if (result.icons1.length)
                fputs(", ", stdout);
            ffStrbufWriteTo(&result.icons2, stdout);
        }
        putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result.icons1, "icons1"),
            FF_FORMAT_ARG(result.icons2, "icons2"),
        }));
    }
    success = true;

exit:
    ffStrbufDestroy(&result.icons1);
    ffStrbufDestroy(&result.icons2);

    return success;
}

void ffParseIconsJsonObject(FFIconsOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_ICONS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateIconsJsonConfig(FFIconsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateIconsJsonResult(FF_MAYBE_UNUSED FFIconsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFIconsResult result = {
        .icons1 = ffStrbufCreate(),
        .icons2 = ffStrbufCreate()
    };
    const char* error = ffDetectIcons(&result);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* icons = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, icons, "icons1", &result.icons1);
    yyjson_mut_obj_add_strbuf(doc, icons, "icons2", &result.icons2);
    success = true;

exit:
    ffStrbufDestroy(&result.icons1);
    ffStrbufDestroy(&result.icons2);

    return success;
}

void ffInitIconsOptions(FFIconsOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyIconsOptions(FFIconsOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffIconsModuleInfo = {
    .name = FF_ICONS_MODULE_NAME,
    .description = "Print icon style name",
    .initOptions = (void*) ffInitIconsOptions,
    .destroyOptions = (void*) ffDestroyIconsOptions,
    .parseJsonObject = (void*) ffParseIconsJsonObject,
    .printModule = (void*) ffPrintIcons,
    .generateJsonResult = (void*) ffGenerateIconsJsonResult,
    .generateJsonConfig = (void*) ffGenerateIconsJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Icons part 1", "icons1"},
        {"Icons part 2", "icons2"},
    }))
};
