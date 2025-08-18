#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/initsystem/initsystem.h"
#include "modules/initsystem/initsystem.h"
#include "util/stringUtils.h"

#define FF_INITSYSTEM_DISPLAY_NAME "Init System"

bool ffPrintInitSystem(FFInitSystemOptions* options)
{
    bool success = false;
    FFInitSystemResult result = {
        .name = ffStrbufCreate(),
        .exe = ffStrbufCreate(),
        .version = ffStrbufCreate(),
        .pid = 1,
    };

    const char* error = ffDetectInitSystem(&result);

    if(error)
    {
        ffPrintError(FF_INITSYSTEM_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        goto exit;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_INITSYSTEM_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufWriteTo(&result.name, stdout);
        if (result.version.length)
            printf(" %s\n", result.version.chars);
        else
            putchar('\n');
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_INITSYSTEM_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result.name, "name"),
            FF_FORMAT_ARG(result.exe, "exe"),
            FF_FORMAT_ARG(result.version, "version"),
            FF_FORMAT_ARG(result.pid, "pid"),
        }));
    }
    success = true;

exit:
    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.exe);
    ffStrbufDestroy(&result.version);

    return success;
}

void ffParseInitSystemJsonObject(FFInitSystemOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_INITSYSTEM_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateInitSystemJsonConfig(FFInitSystemOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateInitSystemJsonResult(FF_MAYBE_UNUSED FFInitSystemOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFInitSystemResult result = {
        .name = ffStrbufCreate(),
        .exe = ffStrbufCreate(),
        .version = ffStrbufCreate(),
        .pid = 1,
    };

    const char* error = ffDetectInitSystem(&result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        goto exit;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "name", &result.name);
    yyjson_mut_obj_add_strbuf(doc, obj, "exe", &result.exe);
    yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);
    yyjson_mut_obj_add_uint(doc, obj, "pid", result.pid);
    success = true;

exit:
    ffStrbufDestroy(&result.name);
    ffStrbufDestroy(&result.exe);
    ffStrbufDestroy(&result.version);
    return success;
}

void ffInitInitSystemOptions(FFInitSystemOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰿄");
}

void ffDestroyInitSystemOptions(FFInitSystemOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffInitSystemModuleInfo = {
    .name = FF_INITSYSTEM_MODULE_NAME,
    .description = "Print init system (pid 1) name and version",
    .initOptions = (void*) ffInitInitSystemOptions,
    .destroyOptions = (void*) ffDestroyInitSystemOptions,
    .parseJsonObject = (void*) ffParseInitSystemJsonObject,
    .printModule = (void*) ffPrintInitSystem,
    .generateJsonResult = (void*) ffGenerateInitSystemJsonResult,
    .generateJsonConfig = (void*) ffGenerateInitSystemJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Init system name", "name"},
        {"Init system exe path", "exe"},
        {"Init system version path", "version"},
        {"Init system pid", "pid"},
    }))
};
