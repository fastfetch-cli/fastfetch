#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "detection/cpuusage/cpuusage.h"
#include "modules/cpuusage/cpuusage.h"
#include "util/stringUtils.h"

#define FF_CPUUSAGE_DISPLAY_NAME "CPU Usage"

bool ffPrintCPUUsage(FFCPUUsageOptions* options)
{
    FF_LIST_AUTO_DESTROY percentages = ffListCreate(sizeof(double));
    const char* error = ffGetCpuUsageResult(options, &percentages);

    if(error)
    {
        ffPrintError(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    double maxValue = -999, minValue = 999, sumValue = 0;
    uint32_t maxIndex = 999, minIndex = 999;

    uint32_t index = 0, valueCount = 0;
    FF_LIST_FOR_EACH(double, percent, percentages)
    {
        if (*percent == *percent)
        {
            sumValue += *percent;

            #if WIN32
            // Windows may return values greater than 100%, cap them to 100%
            if (*percent > 100) *percent = 100;
            #endif

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
    #if WIN32
    // See above comment
    if (avgValue > 100) avgValue = 100;
    #endif

    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
        if (!options->separate)
        {
            if(percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                ffPercentAppendBar(&str, avgValue, options->percent, &options->moduleArgs);
            if(percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
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
        FF_STRBUF_AUTO_DESTROY avgNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&avgNum, avgValue, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY avgBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&avgBar, avgValue, options->percent, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY minNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&minNum, minValue, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY minBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&minBar, minValue, options->percent, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY maxNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&maxNum, maxValue, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY maxBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&maxBar, maxValue, options->percent, &options->moduleArgs);

        FF_PRINT_FORMAT_CHECKED(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(avgNum, "avg"),
            FF_FORMAT_ARG(maxNum, "max"),
            FF_FORMAT_ARG(maxIndex, "max-index"),
            FF_FORMAT_ARG(minNum, "min"),
            FF_FORMAT_ARG(minIndex, "min-index"),
            FF_FORMAT_ARG(avgBar, "avg-bar"),
            FF_FORMAT_ARG(maxBar, "max-bar"),
            FF_FORMAT_ARG(minBar, "min-bar"),
        }));
    }

    return true;
}

void ffParseCPUUsageJsonObject(FFCPUUsageOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "separate"))
        {
            options->separate = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "waitTime"))
        {
            options->waitTime = (uint32_t) yyjson_get_uint(val);
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_CPUUSAGE_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateCPUUsageJsonConfig(FFCPUUsageOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "separate", options->separate);

    ffPercentGenerateJsonConfig(doc, module, options->percent);

    yyjson_mut_obj_add_uint(doc, module, "waitTime", options->waitTime);
}

bool ffGenerateCPUUsageJsonResult(FFCPUUsageOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY percentages = ffListCreate(sizeof(double));
    const char* error = ffGetCpuUsageResult(options, &percentages);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }
    yyjson_mut_val* result = yyjson_mut_obj_add_arr(doc, module, "result");
    FF_LIST_FOR_EACH(double, percent, percentages)
    {
        yyjson_mut_arr_add_real(doc, result, *percent);
    }

    return true;
}

void ffInitCPUUsageOptions(FFCPUUsageOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󰓅");
    options->separate = false;
    options->percent = (FFPercentageModuleConfig) { 50, 80, 0 };
    options->waitTime = 200;
}

void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffCPUUsageModuleInfo = {
    .name = FF_CPUUSAGE_MODULE_NAME,
    .description = "Print CPU usage. Costs some time to collect data",
    .initOptions = (void*) ffInitCPUUsageOptions,
    .destroyOptions = (void*) ffDestroyCPUUsageOptions,
    .parseJsonObject = (void*) ffParseCPUUsageJsonObject,
    .printModule = (void*) ffPrintCPUUsage,
    .generateJsonResult = (void*) ffGenerateCPUUsageJsonResult,
    .generateJsonConfig = (void*) ffGenerateCPUUsageJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"CPU usage (percentage num, average)", "avg"},
        {"CPU usage (percentage num, maximum)", "max"},
        {"CPU core index of maximum usage", "max-index"},
        {"CPU usage (percentage num, minimum)", "min"},
        {"CPU core index of minimum usage", "min-index"},
        {"CPU usage (percentage bar, average)", "avg-bar"},
        {"CPU usage (percentage bar, maximum)", "max-bar"},
        {"CPU usage (percentage bar, minimum)", "min-bar"},
    }))
};
