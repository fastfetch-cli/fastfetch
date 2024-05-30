#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "detection/cpuusage/cpuusage.h"
#include "modules/cpuusage/cpuusage.h"
#include "util/stringUtils.h"

#define FF_CPUUSAGE_DISPLAY_NAME "CPU Usage"
#define FF_CPUUSAGE_NUM_FORMAT_ARGS 5

void ffPrintCPUUsage(FFCPUUsageOptions* options)
{
    FF_LIST_AUTO_DESTROY percentages = ffListCreate(sizeof(double));
    const char* error = ffGetCpuUsageResult(&percentages);

    if(error)
    {
        ffPrintError(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }

    double maxValue = -999, minValue = 999, sumValue = 0;
    uint32_t maxIndex = 999, minIndex = 999;

    uint32_t index = 0, valueCount = 0;
    FF_LIST_FOR_EACH(double, percent, percentages)
    {
        if (*percent == *percent)
        {
            sumValue += *percent;
            if (*percent > maxValue)
            {
                maxValue = *percent;
                maxIndex = index;
            }
            if (*percent < minValue)
            {
                minValue = *percent;
                minIndex = index;
            }
            ++valueCount;
        }
        ++index;
    }
    double avgValue = sumValue / (double) valueCount;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
        if (!options->separate)
        {
            if(instance.config.display.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                ffPercentAppendBar(&str, avgValue, options->percent, &options->moduleArgs);
            if(instance.config.display.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                if(str.length > 0)
                    ffStrbufAppendC(&str, ' ');
                ffPercentAppendNum(&str, avgValue, options->percent, str.length > 0, &options->moduleArgs);
            }
        }
        else
        {
            FF_LIST_FOR_EACH(double, percent, percentages)
            {
                if(str.length > 0)
                    ffStrbufAppendC(&str, ' ');
                ffPercentAppendNum(&str, *percent, options->percent, false, &options->moduleArgs);
            }
        }
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY avgStr = ffStrbufCreate();
        ffPercentAppendNum(&avgStr, avgValue, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY minStr = ffStrbufCreate();
        ffPercentAppendNum(&minStr, minValue, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY maxStr = ffStrbufCreate();
        ffPercentAppendNum(&maxStr, maxValue, options->percent, false, &options->moduleArgs);
        FF_PRINT_FORMAT_CHECKED(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_CPUUSAGE_NUM_FORMAT_ARGS, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &avgStr, "avg"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &maxStr, "max"},
            {FF_FORMAT_ARG_TYPE_UINT, &maxIndex, "max-index"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &minStr, "min"},
            {FF_FORMAT_ARG_TYPE_UINT, &minIndex, "min-index"},
        }));
    }
}

bool ffParseCPUUsageCommandOptions(FFCPUUsageOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CPUUSAGE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "separate"))
    {
        options->separate = ffOptionParseBoolean(value);
        return true;
    }

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
}

void ffParseCPUUsageJsonObject(FFCPUUsageOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "separate"))
        {
            options->separate = yyjson_get_bool(val);
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_CPUUSAGE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateCPUUsageJsonConfig(FFCPUUsageOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyCPUUsageOptions))) FFCPUUsageOptions defaultOptions;
    ffInitCPUUsageOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    if (options->separate != defaultOptions.separate)
        yyjson_mut_obj_add_bool(doc, module, "separate", options->separate);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
}

void ffGenerateCPUUsageJsonResult(FF_MAYBE_UNUSED FFCPUUsageOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY percentages = ffListCreate(sizeof(double));
    const char* error = ffGetCpuUsageResult(&percentages);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }
    yyjson_mut_val* result = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(double, percent, percentages)
    {
        yyjson_mut_arr_add_real(doc, result, *percent);
    }
}

void ffPrintCPUUsageHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_CPUUSAGE_MODULE_NAME, "{1}", FF_CPUUSAGE_NUM_FORMAT_ARGS, ((const char* []) {
        "CPU usage (percentage, average) - avg",
        "CPU usage (percentage, maximum) - max",
        "CPU core index of maximum usage - max-index",
        "CPU usage (percentage, minimum) - min",
        "CPU core index of minimum usage - min-index",
    }));
}

void ffInitCPUUsageOptions(FFCPUUsageOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_CPUUSAGE_MODULE_NAME,
        "Print CPU usage. Costs some time to collect data",
        ffParseCPUUsageCommandOptions,
        ffParseCPUUsageJsonObject,
        ffPrintCPUUsage,
        ffGenerateCPUUsageJsonResult,
        ffPrintCPUUsageHelpFormat,
        ffGenerateCPUUsageJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
    options->separate = false;
    options->percent = (FFColorRangeConfig) { 50, 80 };
}

void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
