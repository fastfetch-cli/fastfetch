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
        ffPrintFormat(FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_CPUUSAGE_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_DOUBLE, &percentage}
        });
    }
}

void ffInitCPUUsageOptions(FFCPUUsageOptions* options)
{
    options->moduleName = FF_CPUUSAGE_MODULE_NAME;
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

void ffParseCPUUsageJsonObject(yyjson_val* module)
{
    FFCPUUsageOptions __attribute__((__cleanup__(ffDestroyCPUUsageOptions))) options;
    ffInitCPUUsageOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(FF_CPUUSAGE_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCPUUsage(&options);
}
