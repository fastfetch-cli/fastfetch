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
            ffPrintLogoAndKey(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

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
                ffStrbufAppendF(&str, " @ %.*f GHz", options->freqNdigits, cpu.frequencyMax);

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

    if (ffStrEqualsIgnCase(subKey, "freq-ndigits"))
    {
        options->freqNdigits = (uint8_t) ffOptionParseUInt32(key, value);
        return true;
    }

    return false;
}

void ffParseCPUJsonObject(FFCPUOptions* options, yyjson_val* module)
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

        if (ffStrEqualsIgnCase(key, "temp"))
        {
            options->temp = yyjson_get_bool(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "freqNdigits"))
        {
            options->freqNdigits = (uint8_t) yyjson_get_uint(val);
            continue;
        }

        ffPrintError(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}

void ffGenerateCPUJsonResult(FFCPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
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
        yyjson_mut_obj_add_str(doc, module, "error", error);
    }
    else if(cpu.vendor.length == 0 && cpu.name.length == 0 && cpu.coresOnline <= 1)
    {
        yyjson_mut_obj_add_str(doc, module, "error", "No CPU detected");
    }
    else
    {
        yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
        yyjson_mut_obj_add_strbuf(doc, obj, "cpu", &cpu.name);
        yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &cpu.vendor);

        yyjson_mut_val* cores = yyjson_mut_obj_add_obj(doc, obj, "cores");
        yyjson_mut_obj_add_uint(doc, cores, "physical", cpu.coresPhysical);
        yyjson_mut_obj_add_uint(doc, cores, "logical", cpu.coresLogical);
        yyjson_mut_obj_add_uint(doc, cores, "online", cpu.coresOnline);

        yyjson_mut_val* frequency = yyjson_mut_obj_add_obj(doc, obj, "frequency");
        yyjson_mut_obj_add_real(doc, frequency, "min", cpu.frequencyMin);
        yyjson_mut_obj_add_real(doc, frequency, "max", cpu.frequencyMax);

        yyjson_mut_obj_add_real(doc, obj, "temperature", cpu.temperature);
    }

    ffStrbufDestroy(&cpu.name);
    ffStrbufDestroy(&cpu.vendor);
}

void ffPrintCPUHelpFormat(void)
{
    ffPrintModuleFormatHelp(FF_CPU_MODULE_NAME, "{1} ({5}) @ {7} GHz", FF_CPU_NUM_FORMAT_ARGS, (const char* []) {
        "Name",
        "Vendor",
        "Physical core count",
        "Logical core count",
        "Online core count",
        "Min frequency",
        "Max frequency",
        "Temperature"
    });
}

void ffInitCPUOptions(FFCPUOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_CPU_MODULE_NAME,
        ffParseCPUCommandOptions,
        ffParseCPUJsonObject,
        ffPrintCPU,
        ffGenerateCPUJsonResult,
        ffPrintCPUHelpFormat,
        NULL
    );
    ffOptionInitModuleArg(&options->moduleArgs);
    options->temp = false;
    options->freqNdigits = 2;
}

void ffDestroyCPUOptions(FFCPUOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
