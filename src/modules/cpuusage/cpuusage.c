#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/bar.h"
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
        ffPrintError(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    double maxValue = -999, minValue = 999, sumValue = 0;
    uint32_t maxIndex = 999, minIndex = 999;

    uint32_t index = 0;
    FF_LIST_FOR_EACH(double, percent, percentages)
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
        ++index;
    }
    double avgValue = sumValue / (double) percentages.length;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
        if (!options->separate)
        {
            if(instance.config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                ffAppendPercentBar(&str, avgValue, 0, 50, 80);
            if(instance.config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                if(str.length > 0)
                    ffStrbufAppendC(&str, ' ');
                ffAppendPercentNum(&str, avgValue, 50, 80, str.length > 0);
            }
        }
        else
        {
            FF_LIST_FOR_EACH(double, percent, percentages)
            {
                if(str.length > 0)
                    ffStrbufAppendC(&str, ' ');
                ffAppendPercentNum(&str, *percent, 50, 80, false);
            }
        }
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY avgStr = ffStrbufCreate();
        ffAppendPercentNum(&avgStr, avgValue, 50, 80, false);
        FF_STRBUF_AUTO_DESTROY minStr = ffStrbufCreate();
        ffAppendPercentNum(&minStr, minValue, 50, 80, false);
        FF_STRBUF_AUTO_DESTROY maxStr = ffStrbufCreate();
        ffAppendPercentNum(&maxStr, maxValue, 50, 80, false);
        ffPrintFormat(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_CPUUSAGE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &avgStr},
            {FF_FORMAT_ARG_TYPE_STRBUF, &maxStr},
            {FF_FORMAT_ARG_TYPE_UINT, &maxIndex},
            {FF_FORMAT_ARG_TYPE_STRBUF, &minStr},
            {FF_FORMAT_ARG_TYPE_UINT, &minIndex},
        });
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

        ffPrintError(FF_CPUUSAGE_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
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
    ffPrintModuleFormatHelp(FF_CPUUSAGE_MODULE_NAME, "{1}", FF_CPUUSAGE_NUM_FORMAT_ARGS, (const char* []) {
        "CPU usage (percentage, average)",
        "CPU usage (percentage, maximum)",
        "CPU core index of maximum usage",
        "CPU usage (percentage, minimum)",
        "CPU core index of minimum usage",
    });
}

void ffInitCPUUsageOptions(FFCPUUsageOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_CPUUSAGE_MODULE_NAME,
        ffParseCPUUsageCommandOptions,
        ffParseCPUUsageJsonObject,
        ffPrintCPUUsage,
        ffGenerateCPUUsageJsonResult,
        ffPrintCPUUsageHelpFormat,
        NULL
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
