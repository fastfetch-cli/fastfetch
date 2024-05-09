#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "common/parsing.h"
#include "common/temps.h"
#include "detection/battery/battery.h"
#include "modules/battery/battery.h"
#include "util/stringUtils.h"

#define FF_BATTERY_NUM_FORMAT_ARGS 9

static void printBattery(FFBatteryOptions* options, FFBatteryResult* result, uint8_t index)
{
    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_BATTERY_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
        bool showStatus =
            !(instance.config.display.percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT) &&
            result->status.length > 0 &&
            ffStrbufIgnCaseCompS(&result->status, "Unknown") != 0;

        if(result->capacity >= 0)
        {
            if(instance.config.display.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffPercentAppendBar(&str, result->capacity, options->percent, &options->moduleArgs);
            }

            if(instance.config.display.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                if(str.length > 0)
                    ffStrbufAppendC(&str, ' ');

                ffPercentAppendNum(&str, result->capacity, options->percent, str.length > 0, &options->moduleArgs);
            }
        }

        if(showStatus)
        {
            if(str.length > 0)
                ffStrbufAppendF(&str, " [%s]", result->status.chars);
            else
                ffStrbufAppend(&str, &result->status);
        }

        if(result->temperature == result->temperature) //FF_BATTERY_TEMP_UNSET
        {
            if(str.length > 0)
                ffStrbufAppendS(&str, " - ");

            ffTempsAppendNum(result->temperature, &str, options->tempConfig, &options->moduleArgs);
        }

        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY capacityStr = ffStrbufCreate();
        ffPercentAppendNum(&capacityStr, result->capacity, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY tempStr = ffStrbufCreate();
        ffTempsAppendNum(result->temperature, &tempStr, options->tempConfig, &options->moduleArgs);
        FF_PRINT_FORMAT_CHECKED(FF_BATTERY_MODULE_NAME, index, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_BATTERY_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->manufacturer},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->modelName},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->technology},
            {FF_FORMAT_ARG_TYPE_STRBUF, &capacityStr},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->status},
            {FF_FORMAT_ARG_TYPE_STRBUF, &tempStr},
            {FF_FORMAT_ARG_TYPE_UINT, &result->cycleCount},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->serial},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result->manufactureDate},
        }));
    }
}

void ffPrintBattery(FFBatteryOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBatteryResult));

    const char* error = ffDetectBattery(options, &results);

    if (error)
    {
        ffPrintError(FF_BATTERY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }
    if(results.length == 0)
    {
        ffPrintError(FF_BATTERY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "No batteries found");
        return;
    }

    for(uint32_t i = 0; i < results.length; i++)
    {
        FFBatteryResult* result = ffListGet(&results, i);
        printBattery(options, result, (uint8_t) i);
    }

    FF_LIST_FOR_EACH(FFBatteryResult, result, results)
    {
        ffStrbufDestroy(&result->manufacturer);
        ffStrbufDestroy(&result->modelName);
        ffStrbufDestroy(&result->technology);
        ffStrbufDestroy(&result->status);
        ffStrbufDestroy(&result->serial);
        ffStrbufDestroy(&result->manufactureDate);
    }
}

bool ffParseBatteryCommandOptions(FFBatteryOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BATTERY_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffTempsParseCommandOptions(key, subKey, value, &options->temp, &options->tempConfig))
        return true;

    #ifdef _WIN32
        if (ffStrEqualsIgnCase(subKey, "use-setup-api"))
        {
            options->useSetupApi = ffOptionParseBoolean(value);
            return true;
        }
    #endif

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
}

void ffParseBatteryJsonObject(FFBatteryOptions* options, yyjson_val* module)
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

        #ifdef _WIN32
        if (ffStrEqualsIgnCase(key, "useSetupApi"))
        {
            options->useSetupApi = yyjson_get_bool(val);
            continue;
        }
        #endif

        if (ffTempsParseJsonObject(key, val, &options->temp, &options->tempConfig))
            continue;

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_BATTERY_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateBatteryJsonConfig(FFBatteryOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyBatteryOptions))) FFBatteryOptions defaultOptions;
    ffInitBatteryOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    #ifdef _WIN32
    if (defaultOptions.useSetupApi != options->useSetupApi)
        yyjson_mut_obj_add_bool(doc, module, "useSetupApi", options->useSetupApi);
    #endif

    ffTempsGenerateJsonConfig(doc, module, defaultOptions.temp, defaultOptions.tempConfig, options->temp, options->tempConfig);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
}

void ffGenerateBatteryJsonResult(FFBatteryOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBatteryResult));

    const char* error = ffDetectBattery(options, &results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFBatteryResult, battery, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_real(doc, obj, "capacity", battery->capacity);
        yyjson_mut_obj_add_strbuf(doc, obj, "manufacturer", &battery->manufacturer);
        yyjson_mut_obj_add_strbuf(doc, obj, "manufactureDate", &battery->manufactureDate);
        yyjson_mut_obj_add_strbuf(doc, obj, "modelName", &battery->modelName);
        yyjson_mut_obj_add_strbuf(doc, obj, "status", &battery->status);
        yyjson_mut_obj_add_strbuf(doc, obj, "technology", &battery->technology);
        yyjson_mut_obj_add_strbuf(doc, obj, "serial", &battery->serial);
        yyjson_mut_obj_add_real(doc, obj, "temperature", battery->temperature);
        yyjson_mut_obj_add_uint(doc, obj, "cycleCount", battery->cycleCount);
    }

    FF_LIST_FOR_EACH(FFBatteryResult, battery, results)
    {
        ffStrbufDestroy(&battery->manufacturer);
        ffStrbufDestroy(&battery->manufactureDate);
        ffStrbufDestroy(&battery->modelName);
        ffStrbufDestroy(&battery->technology);
        ffStrbufDestroy(&battery->status);
        ffStrbufDestroy(&battery->serial);
    }
}

void ffPrintBatteryHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_BATTERY_MODULE_NAME, "{4}, {5}", FF_BATTERY_NUM_FORMAT_ARGS, ((const char* []) {
        "Battery manufactor",
        "Battery model",
        "Battery technology",
        "Battery capacity (percentage)",
        "Battery status",
        "Battery temperature (formatted)",
        "Battery cycle count",
        "Battery serial number",
        "Battery manufactor date",
    }));
}

void ffInitBatteryOptions(FFBatteryOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BATTERY_MODULE_NAME,
        "Print battery capacity, status, etc",
        ffParseBatteryCommandOptions,
        ffParseBatteryJsonObject,
        ffPrintBattery,
        ffGenerateBatteryJsonResult,
        ffPrintBatteryHelpFormat,
        ffGenerateBatteryJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
    options->temp = false;
    options->tempConfig = (FFColorRangeConfig) { 60, 80 };
    options->percent = (FFColorRangeConfig) { 50, 20 };

    #ifdef _WIN32
        options->useSetupApi = false;
    #endif
}

void ffDestroyBatteryOptions(FFBatteryOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
