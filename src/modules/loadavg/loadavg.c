#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "detection/loadavg/loadavg.h"
#include "detection/cpu/cpu.h"
#include "modules/loadavg/loadavg.h"
#include "util/stringUtils.h"

bool ffPrintLoadavg(FFLoadavgOptions* options)
{
    double result[3] = { 0.0 / 0.0, 0.0 / 0.0, 0.0 / 0.0 };

    const char* error = ffDetectLoadavg(result);
    if(error)
    {
        ffPrintError(FF_LOADAVG_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        if (options->compact)
        {
            ffPrintLogoAndKey(FF_LOADAVG_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            printf("%.*f, %.*f, %.*f\n", options->ndigits, result[0], options->ndigits, result[1], options->ndigits, result[2]);
        }
        else
        {
            FFCPUResult cpu = {
                .temperature = FF_CPU_TEMP_UNSET,
                .frequencyMax = 0,
                .frequencyBase = 0,
                .name = ffStrbufCreate(),
                .vendor = ffStrbufCreate(),
            };
            ffDetectCPU(&(FFCPUOptions) {}, &cpu);

            FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
            for (uint32_t index = 0; index < 3; index++)
            {
                uint32_t duration = index == 0 ? 1 : index == 1 ? 5 : 15;
                if (options->moduleArgs.key.length == 0)
                {
                    ffStrbufSetF(&buffer, "%s (%d min)", FF_LOADAVG_MODULE_NAME, duration);
                }
                else
                {
                    ffStrbufClear(&buffer);
                    FF_PARSE_FORMAT_STRING_CHECKED(&buffer, &options->moduleArgs.key, ((FFformatarg[]) {
                        FF_FORMAT_ARG(index, "index"),
                        FF_FORMAT_ARG(duration, "duration"),
                        FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
                    }));
                }

                ffPrintLogoAndKey(buffer.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

                ffStrbufClear(&buffer);
                double percent = result[index] * 100 / cpu.coresOnline;
                FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

                if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
                    ffPercentAppendBar(&buffer, percent, options->percent, &options->moduleArgs);

                if (!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                {
                    if (buffer.length > 0)
                        ffStrbufAppendC(&buffer, ' ');
                    ffStrbufAppendF(&buffer, "%.*f", options->ndigits, result[index]);
                }

                if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
                {
                    if (buffer.length > 0)
                        ffStrbufAppendC(&buffer, ' ');
                    ffPercentAppendNum(&buffer, percent, options->percent, buffer.length > 0, &options->moduleArgs);
                }

                ffStrbufPutTo(&buffer, stdout);
            }
        }
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_LOADAVG_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
            FF_FORMAT_ARG(result[0], "loadavg1"),
            FF_FORMAT_ARG(result[1], "loadavg2"),
            FF_FORMAT_ARG(result[2], "loadavg3"),
        }));
    }

    return true;
}

void ffParseLoadavgJsonObject(FFLoadavgOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "ndigits"))
        {
            options->ndigits = (uint8_t) yyjson_get_uint(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "compact"))
        {
            options->compact = yyjson_get_bool(val);
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_LOADAVG_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateLoadavgJsonConfig(FFLoadavgOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_uint(doc, module, "ndigits", options->ndigits);

    yyjson_mut_obj_add_bool(doc, module, "compact", options->compact);

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateLoadavgJsonResult(FF_MAYBE_UNUSED FFLoadavgOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    double result[3] = { 0.0 / 0.0, 0.0 / 0.0, 0.0 / 0.0 };

    const char* error = ffDetectLoadavg(result);
    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");
    for (size_t i = 0; i < 3; i++)
        yyjson_mut_arr_add_real(doc, arr, result[i]);

    return true;
}

void ffInitLoadavgOptions(FFLoadavgOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->percent = (FFPercentageModuleConfig) { 50, 80, 0 };
    options->ndigits = 2;
    options->compact = true;
}

void ffDestroyLoadavgOptions(FFLoadavgOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffLoadavgModuleInfo = {
    .name = FF_LOADAVG_MODULE_NAME,
    .description = "Print system load averages",
    .initOptions = (void*) ffInitLoadavgOptions,
    .destroyOptions = (void*) ffDestroyLoadavgOptions,
    .parseJsonObject = (void*) ffParseLoadavgJsonObject,
    .printModule = (void*) ffPrintLoadavg,
    .generateJsonResult = (void*) ffGenerateLoadavgJsonResult,
    .generateJsonConfig = (void*) ffGenerateLoadavgJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Load average over 1min", "loadavg1"},
        {"Load average over 5min", "loadavg2"},
        {"Load average over 15min", "loadavg3"},
    }))
};
