#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "detection/cpu/cpu.h"
#include "modules/cpu/cpu.h"
#include "util/stringUtils.h"

#define FF_CPU_NUM_FORMAT_ARGS 8

void ffPrintCPU(FFCPUOptions* options)
{
    FFCPUResult cpu;
    cpu.temperature = FF_CPU_TEMP_UNSET;
    cpu.coresPhysical = cpu.coresLogical = cpu.coresOnline = 0;
    cpu.frequencyMax = cpu.frequencyMin = 0;
    ffStrbufInit(&cpu.name);
    ffStrbufInit(&cpu.vendor);

    const char* error = ffDetectCPU(options, &cpu);

    if(error)
    {
        ffPrintError(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
    }
    else if(cpu.vendor.length == 0 && cpu.name.length == 0 && cpu.coresOnline <= 1)
    {
        ffPrintError(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, "No CPU detected");
    }
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_CPU_MODULE_NAME, 0, &options->moduleArgs.key, &options->moduleArgs.keyColor);

            FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();

            if(cpu.name.length > 0)
                ffStrbufAppend(&str, &cpu.name);
            else if(cpu.vendor.length > 0)
            {
                ffStrbufAppend(&str, &cpu.vendor);
                ffStrbufAppendS(&str, " CPU");
            }
            else
                ffStrbufAppendS(&str, "Unknown");

            if(cpu.coresOnline > 1)
                ffStrbufAppendF(&str, " (%u)", cpu.coresOnline);

            if(cpu.frequencyMax > 0.0)
                ffStrbufAppendF(&str, " @ %.9g GHz", cpu.frequencyMax);

            if(cpu.temperature == cpu.temperature) //FF_CPU_TEMP_UNSET
            {
                ffStrbufAppendS(&str, " - ");
                ffParseTemperature(cpu.temperature, &str);
            }

            ffStrbufPutTo(&str, stdout);
        }
        else
        {
            ffPrintFormat(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_CPU_NUM_FORMAT_ARGS, (FFformatarg[]){
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

    ffStrbufDestroy(&cpu.name);
    ffStrbufDestroy(&cpu.vendor);
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

    if (ffStrEqualsIgnCase(subKey, "temp"))
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

void ffParseCPUJsonObject(yyjson_val* module)
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
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options.moduleArgs))
                continue;

            if (ffStrEqualsIgnCase(key, "temp"))
            {
                options.temp = yyjson_get_bool(val);
                continue;
            }

            ffPrintError(FF_CPU_MODULE_NAME, 0, &options.moduleArgs, "Unknown JSON key %s", key);
        }
    }

    ffPrintCPU(&options);
}
