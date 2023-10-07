#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/bar.h"
#include "detection/cpuusage/cpuusage.h"
#include "modules/cpuusage/cpuusage.h"
#include "util/stringUtils.h"

#define FF_CPUUSAGE_DISPLAY_NAME "CPU Usage"
#define FF_CPUUSAGE_NUM_FORMAT_ARGS 1

void ffPrintCPUUsage(FFCPUUsageOptions* options)
{
    double percentage = 0.0/0.0;
    const char* error = ffGetCpuUsageResult(&percentage);

    if(error)
    {
        ffPrintError(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();
        if(instance.config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffAppendPercentBar(&str, percentage, 0, 50, 80);
        if(instance.config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
        {
            if(str.length > 0)
                ffStrbufAppendC(&str, ' ');
            ffAppendPercentNum(&str, percentage, 50, 80, str.length > 0);
        }
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY percentageStr = ffStrbufCreate();
        ffAppendPercentNum(&percentageStr, percentage, 50, 80, false);
        ffPrintFormat(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_CPUUSAGE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &percentageStr}
        });
    }
}

void ffInitCPUUsageOptions(FFCPUUsageOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_CPUUSAGE_MODULE_NAME, ffParseCPUUsageCommandOptions, ffParseCPUUsageJsonObject, ffPrintCPUUsage, ffGenerateCPUUsageJson, ffPrintCPUUsageHelpFormat);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParseCPUUsageCommandOptions(FFCPUUsageOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CPUUSAGE_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyCPUUsageOptions(FFCPUUsageOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
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

        ffPrintError(FF_CPUUSAGE_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateCPUUsageJson(FF_MAYBE_UNUSED FFCPUUsageOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    double percentage = 0.0/0.0;
    const char* error = ffGetCpuUsageResult(&percentage);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }
    yyjson_mut_obj_add_real(doc, module, "result", percentage);
}

void ffPrintCPUUsageHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_CPUUSAGE_MODULE_NAME, "{1}", FF_CPUUSAGE_NUM_FORMAT_ARGS, (const char* []) {
        "CPU usage (percentage)"
    });
}
