#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/weather/weather.h"
#include "modules/weather/weather.h"
#include "util/stringUtils.h"

void ffPrintWeather(FFWeatherOptions* options)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    const char* error = ffDetectWeather(options, &result);

    if(error)
    {
        ffPrintError(FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }


    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(result, "result"),
        }));
    }
}

void ffParseWeatherJsonObject(FFWeatherOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "location"))
        {
            ffStrbufSetJsonVal(&options->location, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "outputFormat"))
        {
            ffStrbufSetJsonVal(&options->outputFormat, val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "timeout"))
        {
            options->timeout = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        ffPrintError(FF_WEATHER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateWeatherJsonConfig(FFWeatherOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyWeatherOptions))) FFWeatherOptions defaultOptions;
    ffInitWeatherOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (!ffStrbufEqual(&options->location, &defaultOptions.location))
        yyjson_mut_obj_add_strbuf(doc, module, "location", &options->location);

    if (!ffStrbufEqual(&options->outputFormat, &defaultOptions.outputFormat))
        yyjson_mut_obj_add_strbuf(doc, module, "outputFormat", &options->outputFormat);

    if (options->timeout != defaultOptions.timeout)
        yyjson_mut_obj_add_uint(doc, module, "timeout", options->timeout);
}

void ffGenerateWeatherJsonResult(FFWeatherOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    const char* error = ffDetectWeather(options, &result);

    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_obj_add_strbuf(doc, module, "result", &result);
}

void ffInitWeatherOptions(FFWeatherOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰖙");

    ffStrbufInit(&options->location);
    ffStrbufInitStatic(&options->outputFormat, "%t+-+%C+(%l)");
    options->timeout = 0;
}

void ffDestroyWeatherOptions(FFWeatherOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);

    ffStrbufDestroy(&options->outputFormat);
}

FFModuleBaseInfo ffWeatherModuleInfo = {
    .name = FF_WEATHER_MODULE_NAME,
    .description = "Print weather information",
    .initOptions = (void*) ffInitWeatherOptions,
    .destroyOptions = (void*) ffDestroyWeatherOptions,
    .parseJsonObject = (void*) ffParseWeatherJsonObject,
    .printModule = (void*) ffPrintWeather,
    .generateJsonResult = (void*) ffGenerateWeatherJsonResult,
    .generateJsonConfig = (void*) ffGenerateWeatherJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Weather result", "result"},
    }))
};
