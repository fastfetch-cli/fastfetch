#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/cpu/cpu.h"
#include "modules/cpu/cpu.h"

#define FF_CPU_NUM_FORMAT_ARGS 8

void ffPrintCPU(FFinstance* instance, FFCPUOptions* options)
{
    FFCPUResult cpu;
    cpu.temperature = FF_CPU_TEMP_UNSET;
    cpu.coresPhysical = cpu.coresLogical = cpu.coresOnline = 0;
    cpu.frequencyMax = cpu.frequencyMin = 0;
    ffStrbufInit(&cpu.name);
    ffStrbufInit(&cpu.vendor);

    ffDetectCPU(instance, options, &cpu);

    if(cpu.vendor.length == 0 && cpu.name.length == 0 && cpu.coresOnline <= 1)
    {
        ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &options->moduleArgs, "No CPU detected");
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_CPU_MODULE_NAME, 0, &options->moduleArgs.key);

        if(cpu.name.length > 0)
            ffStrbufWriteTo(&cpu.name, stdout);
        else if(cpu.vendor.length > 0)
        {
            ffStrbufWriteTo(&cpu.vendor, stdout);
            fputs(" CPU", stdout);
        }
        else
            fputs("CPU", stdout);

        if(cpu.coresOnline > 1)
            printf(" (%u)", cpu.coresOnline);

        if(cpu.frequencyMax > 0.0)
            printf(" @ %.9g GHz", cpu.frequencyMax);

        if(cpu.temperature == cpu.temperature) //FF_CPU_TEMP_UNSET
            printf(" - %.1f°C", cpu.temperature);

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_CPU_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &cpu.name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &cpu.vendor},
            {FF_FORMAT_ARG_TYPE_UINT16, &cpu.coresPhysical},
            {FF_FORMAT_ARG_TYPE_UINT16, &cpu.coresLogical},
            {FF_FORMAT_ARG_TYPE_UINT16, &cpu.coresOnline},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu.frequencyMin},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu.frequencyMax},
            {FF_FORMAT_ARG_TYPE_DOUBLE, &cpu.temperature}
        });
    }
}

void ffInitCPUOptions(FFCPUOptions* options)
{
    options->moduleName = FF_CPU_MODULE_NAME;
    ffOptionInitModuleArg(&options->moduleArgs);
    options->temp = false;
}

bool ffParseCPUCommandOptions(FFCPUOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_CPU_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (strcasecmp(subKey, "temp") == 0)
    {
        options->temp = ffOptionParseBoolean(value);
        return true;
    }

    return false;
}

void ffDestroyCPUOptions(FFCPUOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseCPUJsonObject(FFinstance* instance, yyjson_val* module)
{
    FFCPUOptions __attribute__((__cleanup__(ffDestroyCPUOptions))) options;
    ffInitCPUOptions(&options);

    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(strcasecmp(key, "type") == 0)
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (strcasecmp(key, "temp") == 0)
            {
                options.temp = yyjson_get_bool(val);
                continue;
            }

            ffPrintError(instance, FF_CPU_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCPU(instance, &options);
}
