#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/temps.h"
#include "detection/cpu/cpu.h"
#include "modules/cpu/cpu.h"
#include "util/stringUtils.h"

#define FF_CPU_NUM_FORMAT_ARGS 10

static int sortCores(const FFCPUCore* a, const FFCPUCore* b)
{
    return (int)b->freq - (int)a->freq;
}

void ffPrintCPU(FFCPUOptions* options)
{
    FFCPUResult cpu = {
        .temperature = FF_CPU_TEMP_UNSET,
        .frequencyMin = 0.0/0.0,
        .frequencyMax = 0.0/0.0,
        .frequencyBase = 0.0/0.0,
        .frequencyBiosLimit = 0.0/0.0,
        .name = ffStrbufCreate(),
        .vendor = ffStrbufCreate()
    };

    const char* error = ffDetectCPU(options, &cpu);

    if(error)
    {
        ffPrintError(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
    }
    else if(cpu.vendor.length == 0 && cpu.name.length == 0 && cpu.coresOnline <= 1)
    {
        ffPrintError(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No CPU detected");
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY coreTypes = ffStrbufCreate();
        if (options->showPeCoreCount)
        {
            uint32_t typeCount = 0;
            while (cpu.coreTypes[typeCount].count != 0 && typeCount < sizeof(cpu.coreTypes) / sizeof(cpu.coreTypes[0])) typeCount++;
            if (typeCount > 0)
            {
                qsort(cpu.coreTypes, typeCount, sizeof(cpu.coreTypes[0]), (void*) sortCores);

                for (uint32_t i = 0; i < typeCount; i++)
                    ffStrbufAppendF(&coreTypes, "%s%u", i == 0 ? "" : "+", cpu.coreTypes[i].count);
            }
        }

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

            if(coreTypes.length > 0)
                ffStrbufAppendF(&str, " (%s)", coreTypes.chars);
            else if(cpu.coresOnline > 1)
                ffStrbufAppendF(&str, " (%u)", cpu.coresOnline);

            double freq = cpu.frequencyBiosLimit;
            if(!(freq > 0.0000001))
                freq = cpu.frequencyMax;
            if(!(freq > 0.0000001))
                freq = cpu.frequencyBase;
            if(freq > 0.0000001)
                ffStrbufAppendF(&str, " @ %.*f GHz", options->freqNdigits, freq);

            if(cpu.temperature == cpu.temperature) //FF_CPU_TEMP_UNSET
            {
                ffStrbufAppendS(&str, " - ");
                ffTempsAppendNum(cpu.temperature, &str, options->tempConfig, &options->moduleArgs);
            }

            ffStrbufPutTo(&str, stdout);
        }
        else
        {
            char freqBase[32], freqMax[32], freqBioslimit[32];
            if (cpu.frequencyBase > 0)
                snprintf(freqBase, sizeof(freqBase), "%.*f", options->freqNdigits, cpu.frequencyBase);
            else
                freqBase[0] = 0;
            if (cpu.frequencyMax > 0)
                snprintf(freqMax, sizeof(freqMax), "%.*f", options->freqNdigits, cpu.frequencyMax);
            else
                freqMax[0] = 0;
            if (cpu.frequencyBiosLimit > 0)
                snprintf(freqBioslimit, sizeof(freqBioslimit), "%.*f", options->freqNdigits, cpu.frequencyBiosLimit);
            else
                freqBioslimit[0] = 0;

            FF_STRBUF_AUTO_DESTROY tempStr = ffStrbufCreate();
            ffTempsAppendNum(cpu.temperature, &tempStr, options->tempConfig, &options->moduleArgs);
            FF_PRINT_FORMAT_CHECKED(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_CPU_NUM_FORMAT_ARGS, ((FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &cpu.name, "name"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &cpu.vendor, "vendor"},
                {FF_FORMAT_ARG_TYPE_UINT16, &cpu.coresPhysical, "cores-physical"},
                {FF_FORMAT_ARG_TYPE_UINT16, &cpu.coresLogical, "cores-logical"},
                {FF_FORMAT_ARG_TYPE_UINT16, &cpu.coresOnline, "cores-online"},
                {FF_FORMAT_ARG_TYPE_STRING, freqBase, "freq-base"},
                {FF_FORMAT_ARG_TYPE_STRING, freqMax, "freq-max"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &tempStr, "temperature"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &coreTypes, "core-types"},
                {FF_FORMAT_ARG_TYPE_STRING, freqBioslimit, "freq-bios-limit"},
            }));
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

    if (ffTempsParseCommandOptions(key, subKey, value, &options->temp, &options->tempConfig))
        return true;

    if (ffStrEqualsIgnCase(subKey, "freq-ndigits"))
    {
        options->freqNdigits = (uint8_t) ffOptionParseUInt32(key, value);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "show-pe-core-count"))
    {
        options->showPeCoreCount = ffOptionParseBoolean(value);
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

        if (ffTempsParseJsonObject(key, val, &options->temp, &options->tempConfig))
            continue;

        if (ffStrEqualsIgnCase(key, "freqNdigits"))
        {
            options->freqNdigits = (uint8_t) yyjson_get_uint(val);
            continue;
        }

        if (ffStrEqualsIgnCase(key, "showPeCoreCount"))
        {
            options->showPeCoreCount = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateCPUJsonConfig(FFCPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyCPUOptions))) FFCPUOptions defaultOptions;
    ffInitCPUOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    ffTempsGenerateJsonConfig(doc, module, defaultOptions.temp, defaultOptions.tempConfig, options->temp, options->tempConfig);

    if (defaultOptions.freqNdigits != options->freqNdigits)
        yyjson_mut_obj_add_uint(doc, module, "freqNdigits", options->freqNdigits);

    if (defaultOptions.showPeCoreCount != options->showPeCoreCount)
        yyjson_mut_obj_add_bool(doc, module, "showPeCoreCount", options->showPeCoreCount);
}

void ffGenerateCPUJsonResult(FFCPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FFCPUResult cpu = {
        .temperature = FF_CPU_TEMP_UNSET,
        .frequencyMin = 0.0/0.0,
        .frequencyMax = 0.0/0.0,
        .frequencyBase = 0.0/0.0,
        .frequencyBiosLimit = 0.0/0.0,
        .name = ffStrbufCreate(),
        .vendor = ffStrbufCreate()
    };

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
        yyjson_mut_obj_add_real(doc, frequency, "base", cpu.frequencyBase);
        yyjson_mut_obj_add_real(doc, frequency, "max", cpu.frequencyMax);
        yyjson_mut_obj_add_real(doc, frequency, "min", cpu.frequencyMin);
        yyjson_mut_obj_add_real(doc, frequency, "biosLimit", cpu.frequencyBiosLimit);

        yyjson_mut_val* coreTypes = yyjson_mut_obj_add_arr(doc, obj, "coreTypes");
        for (uint32_t i = 0; i < sizeof (cpu.coreTypes) / sizeof (cpu.coreTypes[0]) && cpu.coreTypes[i].count > 0; i++)
        {
            yyjson_mut_val* core = yyjson_mut_arr_add_obj(doc, coreTypes);
            yyjson_mut_obj_add_uint(doc, core, "count", cpu.coreTypes[i].count);
            yyjson_mut_obj_add_uint(doc, core, "freq", cpu.coreTypes[i].freq);
        }

        yyjson_mut_obj_add_real(doc, obj, "temperature", cpu.temperature);
    }

    ffStrbufDestroy(&cpu.name);
    ffStrbufDestroy(&cpu.vendor);
}

void ffPrintCPUHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_CPU_MODULE_NAME, "{1} ({5}) @ {7} GHz", FF_CPU_NUM_FORMAT_ARGS, ((const char* []) {
        "Name - name",
        "Vendor - vendor",
        "Physical core count - cores-physical",
        "Logical core count - cores-logical",
        "Online core count - cores-online",
        "Base frequency - freq-base",
        "Max frequency - freq-max",
        "Temperature (formatted) - temperature",
        "Logical core count grouped by frequency - core-types",
        "Bios limited frequency - freq-bios-limit",
    }));
}

void ffInitCPUOptions(FFCPUOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_CPU_MODULE_NAME,
        "Print CPU name, frequency, etc",
        ffParseCPUCommandOptions,
        ffParseCPUJsonObject,
        ffPrintCPU,
        ffGenerateCPUJsonResult,
        ffPrintCPUHelpFormat,
        ffGenerateCPUJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
    options->temp = false;
    options->tempConfig = (FFColorRangeConfig) { 60, 80 };
    options->freqNdigits = 2;
    options->showPeCoreCount = false;
}

void ffDestroyCPUOptions(FFCPUOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
