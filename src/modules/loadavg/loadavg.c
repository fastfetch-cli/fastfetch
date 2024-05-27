#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/loadavg/loadavg.h"
#include "modules/loadavg/loadavg.h"
#include "util/stringUtils.h"

#define FF_LOADAVG_NUM_FORMAT_ARGS 3

void ffPrintLoadavg(FFLoadavgOptions* options)
{
    double result[3] = { 0.0 / 0.0, 0.0 / 0.0, 0.0 / 0.0 };

    const char* error = ffDetectLoadavg(result);
    if(error)
    {
        ffPrintError(FF_LOADAVG_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_LOADAVG_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        printf("%.*f, %.*f, %.*f\n", options->ndigits, result[0], options->ndigits, result[1], options->ndigits, result[2]);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_LOADAVG_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_LOADAVG_NUM_FORMAT_ARGS, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_DOUBLE, &result[0], "loadavg1"},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &result[1], "loadavg2"},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &result[2], "loadavg3"},
        }));
    }
}

bool ffParseLoadavgCommandOptions(FFLoadavgOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_LOADAVG_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "ndigits"))
    {
        options->ndigits = (uint8_t) ffOptionParseUInt32(key, value);
        return true;
    }

    return false;
}

void ffParseLoadavgJsonObject(FFLoadavgOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "ndigits"))
        {
            options->ndigits = (uint8_t) yyjson_get_uint(val);
            continue;
        }

        ffPrintError(FF_LOADAVG_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateLoadavgJsonConfig(FFLoadavgOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyLoadavgOptions))) FFLoadavgOptions defaultOptions;
    ffInitLoadavgOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (defaultOptions.ndigits != options->ndigits)
        yyjson_mut_obj_add_uint(doc, module, "ndigits", options->ndigits);
}

void ffGenerateLoadavgJsonResult(FF_MAYBE_UNUSED FFLoadavgOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    double result[3] = { 0.0 / 0.0, 0.0 / 0.0, 0.0 / 0.0 };

    const char* error = ffDetectLoadavg(result);
    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    for (size_t i = 0; i < 3; i++)
        yyjson_mut_arr_add_real(doc, arr, result[i]);
}

void ffPrintLoadavgHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_LOADAVG_MODULE_NAME, "{1}, {2}, {3}", FF_LOADAVG_NUM_FORMAT_ARGS, ((const char* []) {
        "Load average over 1min - loadavg1",
        "Load average over 5min - loadavg2",
        "Load average over 15min - loadavg3",
    }));
}

void ffInitLoadavgOptions(FFLoadavgOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_LOADAVG_MODULE_NAME,
        "Print system load averages",
        ffParseLoadavgCommandOptions,
        ffParseLoadavgJsonObject,
        ffPrintLoadavg,
        ffGenerateLoadavgJsonResult,
        ffPrintLoadavgHelpFormat,
        ffGenerateLoadavgJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);

    options->ndigits = 2;
}

void ffDestroyLoadavgOptions(FFLoadavgOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
