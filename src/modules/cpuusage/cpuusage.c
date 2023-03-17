#include "fastfetch.h"
#include "common/printing.h"
#include "common/bar.h"
#include "detection/cpuusage/cpuusage.h"
#include "modules/cpuusage/cpuusage.h"

#define FF_CPUUSAGE_DISPLAY_NAME "CPU Usage"
#define FF_CPUUSAGE_NUM_FORMAT_ARGS 1

void ffPrintCPUUsage(FFinstance* instance, FFCPUUsageOptions* options)
{
    double percentage = 0.0/0.0;
    const char* error = ffGetCpuUsageResult(&percentage);

    if(error)
    {
        ffPrintError(instance, FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs.key);

        FF_STRBUF_AUTO_DESTROY str;
        ffStrbufInit(&str);
        if(instance->config.percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffAppendPercentBar(instance, &str, (uint8_t)percentage, 0, 5, 8);
        if(instance->config.percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
        {
            if(str.length > 0)
                ffStrbufAppendC(&str, ' ');
            ffAppendPercentNum(instance, &str, (uint8_t) percentage, 50, 80, str.length > 0);
        }
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_CPUUSAGE_DISPLAY_NAME, 0, &options->moduleArgs, FF_CPUUSAGE_NUM_FORMAT_ARGS, (FFformatarg[]){
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

#ifdef FF_HAVE_JSONC
void ffParseCPUUsageJsonObject(FFinstance* instance, json_object* module)
{
    FFCPUUsageOptions __attribute__((__cleanup__(ffDestroyCPUUsageOptions))) options;
    ffInitCPUUsageOptions(&options);

    if (module)
    {
        json_object_object_foreach(module, key, val)
        {
            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            ffPrintError(instance, FF_CPUUSAGE_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCPUUsage(instance, &options);
}
#endif
