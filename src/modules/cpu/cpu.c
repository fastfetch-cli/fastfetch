#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/parsing.h"
#include "common/temps.h"
#include "common/frequency.h"
#include "detection/cpu/cpu.h"
#include "modules/cpu/cpu.h"
#include "util/stringUtils.h"

static int sortCores(const FFCPUCore* a, const FFCPUCore* b)
{
    return (int)b->freq - (int)a->freq;
}

bool ffPrintCPU(FFCPUOptions* options)
{
    bool success = false;
    FFCPUResult cpu = {
        .temperature = FF_CPU_TEMP_UNSET,
        .frequencyMax = 0,
        .frequencyBase = 0,
        .name = ffStrbufCreate(),
        .vendor = ffStrbufCreate(),
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
            while (cpu.coreTypes[typeCount].count != 0 && typeCount < ARRAY_SIZE(cpu.coreTypes)) typeCount++;
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

            if(cpu.packages > 1)
                ffStrbufAppendF(&str, "%u x ", cpu.packages);

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

            uint32_t freq = cpu.frequencyMax;
            if(freq == 0)
                freq = cpu.frequencyBase;
            if(freq > 0)
            {
                ffStrbufAppendS(&str, " @ ");
                ffFreqAppendNum(freq, &str);
            }

            if(cpu.temperature != FF_CPU_TEMP_UNSET)
            {
                ffStrbufAppendS(&str, " - ");
                ffTempsAppendNum(cpu.temperature, &str, options->tempConfig, &options->moduleArgs);
            }

            ffStrbufPutTo(&str, stdout);
        }
        else
        {
            FF_STRBUF_AUTO_DESTROY freqBase = ffStrbufCreate();
            ffFreqAppendNum(cpu.frequencyBase, &freqBase);
            FF_STRBUF_AUTO_DESTROY freqMax = ffStrbufCreate();
            ffFreqAppendNum(cpu.frequencyMax, &freqMax);

            FF_STRBUF_AUTO_DESTROY tempStr = ffStrbufCreate();
            ffTempsAppendNum(cpu.temperature, &tempStr, options->tempConfig, &options->moduleArgs);
            FF_PRINT_FORMAT_CHECKED(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
                FF_FORMAT_ARG(cpu.name, "name"),
                FF_FORMAT_ARG(cpu.vendor, "vendor"),
                FF_FORMAT_ARG(cpu.coresPhysical, "cores-physical"),
                FF_FORMAT_ARG(cpu.coresLogical, "cores-logical"),
                FF_FORMAT_ARG(cpu.coresOnline, "cores-online"),
                FF_FORMAT_ARG(freqBase, "freq-base"),
                FF_FORMAT_ARG(freqMax, "freq-max"),
                FF_FORMAT_ARG(tempStr, "temperature"),
                FF_FORMAT_ARG(coreTypes, "core-types"),
                FF_FORMAT_ARG(cpu.packages, "packages"),
                FF_FORMAT_ARG(cpu.march, "march"),
            }));
        }
        success = true;
    }

    ffStrbufDestroy(&cpu.name);
    ffStrbufDestroy(&cpu.vendor);

    return success;
}

void ffParseCPUJsonObject(FFCPUOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffTempsParseJsonObject(key, val, &options->temp, &options->tempConfig))
            continue;

        if (unsafe_yyjson_equals_str(key, "freqNdigits"))
        {
            ffPrintError(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "modules.CPU.freqNdigits has been moved to display.freq.ndigits");
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showPeCoreCount"))
        {
            options->showPeCoreCount = yyjson_get_bool(val);
            continue;
        }

        ffPrintError(FF_CPU_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateCPUJsonConfig(FFCPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    ffTempsGenerateJsonConfig(doc, module, options->temp, options->tempConfig);

    yyjson_mut_obj_add_bool(doc, module, "showPeCoreCount", options->showPeCoreCount);
}

bool ffGenerateCPUJsonResult(FFCPUOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFCPUResult cpu = {
        .temperature = FF_CPU_TEMP_UNSET,
        .frequencyMax = 0,
        .frequencyBase = 0,
        .name = ffStrbufCreate(),
        .vendor = ffStrbufCreate(),
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
        if (cpu.packages == 0)
            yyjson_mut_obj_add_null(doc, obj, "packages");
        else
            yyjson_mut_obj_add_uint(doc, obj, "packages", cpu.packages);

        yyjson_mut_val* cores = yyjson_mut_obj_add_obj(doc, obj, "cores");
        yyjson_mut_obj_add_uint(doc, cores, "physical", cpu.coresPhysical);
        yyjson_mut_obj_add_uint(doc, cores, "logical", cpu.coresLogical);
        yyjson_mut_obj_add_uint(doc, cores, "online", cpu.coresOnline);

        yyjson_mut_val* frequency = yyjson_mut_obj_add_obj(doc, obj, "frequency");
        yyjson_mut_obj_add_uint(doc, frequency, "base", cpu.frequencyBase);
        yyjson_mut_obj_add_uint(doc, frequency, "max", cpu.frequencyMax);

        yyjson_mut_val* coreTypes = yyjson_mut_obj_add_arr(doc, obj, "coreTypes");
        for (uint32_t i = 0; i < ARRAY_SIZE(cpu.coreTypes) && cpu.coreTypes[i].count > 0; i++)
        {
            yyjson_mut_val* core = yyjson_mut_arr_add_obj(doc, coreTypes);
            yyjson_mut_obj_add_uint(doc, core, "count", cpu.coreTypes[i].count);
            yyjson_mut_obj_add_uint(doc, core, "freq", cpu.coreTypes[i].freq);
        }

        if (cpu.temperature != FF_CPU_TEMP_UNSET)
            yyjson_mut_obj_add_real(doc, obj, "temperature", cpu.temperature);
        else
            yyjson_mut_obj_add_null(doc, obj, "temperature");

        if (cpu.march)
            yyjson_mut_obj_add_str(doc, obj, "march", cpu.march);
        else
            yyjson_mut_obj_add_null(doc, obj, "march");

        success = true;
    }

    ffStrbufDestroy(&cpu.name);
    ffStrbufDestroy(&cpu.vendor);

    return success;
}

void ffInitCPUOptions(FFCPUOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
    options->temp = false;
    options->tempConfig = (FFColorRangeConfig) { 60, 80 };
    options->showPeCoreCount = false;
}

void ffDestroyCPUOptions(FFCPUOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffCPUModuleInfo = {
    .name = FF_CPU_MODULE_NAME,
    .description = "Print CPU name, frequency, etc",
    .initOptions = (void*) ffInitCPUOptions,
    .destroyOptions = (void*) ffDestroyCPUOptions,
    .parseJsonObject = (void*) ffParseCPUJsonObject,
    .printModule = (void*) ffPrintCPU,
    .generateJsonResult = (void*) ffGenerateCPUJsonResult,
    .generateJsonConfig = (void*) ffGenerateCPUJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Name", "name"},
        {"Vendor", "vendor"},
        {"Physical core count", "cores-physical"},
        {"Logical core count", "cores-logical"},
        {"Online core count", "cores-online"},
        {"Base frequency (formatted)", "freq-base"},
        {"Max frequency (formatted)", "freq-max"},
        {"Temperature (formatted)", "temperature"},
        {"Logical core count grouped by frequency", "core-types"},
        {"Processor package count", "packages"},
        {"X86-64 CPU microarchitecture", "march"},
    }))
};
