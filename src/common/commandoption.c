#include "commandoption.h"
#include "common/color.h"
#include "common/printing.h"
#include "common/time.h"
#include "common/jsonconfig.h"
#include "fastfetch_datatext.h"
#include "modules/modules.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <inttypes.h>

void ffPrepareCommandOption(FFdata* data)
{
    //If we don't have a custom structure, use the default one
    if(data->structure.length == 0)
        ffStrbufAppendS(&data->structure, FASTFETCH_DATATEXT_STRUCTURE); // Cannot use `ffStrbufSetStatic` here because we will modify the string

    if(ffStrbufContainIgnCaseS(&data->structure, FF_CPUUSAGE_MODULE_NAME))
        ffPrepareCPUUsage();

    if(ffStrbufContainIgnCaseS(&data->structure, FF_DISKIO_MODULE_NAME))
    {
        __attribute__((__cleanup__(ffDestroyDiskIOOptions))) FFDiskIOOptions options;
        ffInitDiskIOOptions(&options);
        ffPrepareDiskIO(&options);
    }

    if(ffStrbufContainIgnCaseS(&data->structure, FF_NETIO_MODULE_NAME))
    {
        __attribute__((__cleanup__(ffDestroyNetIOOptions))) FFNetIOOptions options;
        ffInitNetIOOptions(&options);
        ffPrepareNetIO(&options);
    }

    if(instance.config.general.multithreading)
    {
        if(ffStrbufContainIgnCaseS(&data->structure, FF_PUBLICIP_MODULE_NAME))
        {
            __attribute__((__cleanup__(ffDestroyPublicIpOptions))) FFPublicIPOptions options;
            ffInitPublicIpOptions(&options);
            ffPreparePublicIp(&options);
        }

        if(ffStrbufContainIgnCaseS(&data->structure, FF_WEATHER_MODULE_NAME))
        {
            __attribute__((__cleanup__(ffDestroyWeatherOptions))) FFWeatherOptions options;
            ffInitWeatherOptions(&options);
            ffPrepareWeather(&options);
        }
    }
}

static void genJsonConfig(FFModuleBaseInfo* baseInfo, void* options, yyjson_mut_doc* doc)
{
    yyjson_mut_val* modules = yyjson_mut_obj_get(doc->root, "modules");
    if (!modules)
        modules = yyjson_mut_obj_add_arr(doc, doc->root, "modules");

    yyjson_mut_val* module = yyjson_mut_obj(doc);
    FF_STRBUF_AUTO_DESTROY type = ffStrbufCreateS(baseInfo->name);
    ffStrbufLowerCase(&type);
    yyjson_mut_obj_add_strbuf(doc, module, "type", &type);

    if (baseInfo->generateJsonConfig)
        baseInfo->generateJsonConfig(options, doc, module);

    if (yyjson_mut_obj_size(module) > 1)
        yyjson_mut_arr_add_val(modules, module);
    else
        yyjson_mut_arr_add_strbuf(doc, modules, &type);
}

static void genJsonResult(FFModuleBaseInfo* baseInfo, void* options, yyjson_mut_doc* doc)
{
    yyjson_mut_val* module = yyjson_mut_arr_add_obj(doc, doc->root);
    yyjson_mut_obj_add_str(doc, module, "type", baseInfo->name);
    if (baseInfo->generateJsonResult)
        baseInfo->generateJsonResult(options, doc, module);
    else
        yyjson_mut_obj_add_str(doc, module, "error", "Unsupported for JSON format");
}

static void parseStructureCommand(
    const char* line,
    void (*fn)(FFModuleBaseInfo* baseInfo, void* options, yyjson_mut_doc* jsonDoc),
    yyjson_mut_doc* jsonDoc
)
{
    if(ffCharIsEnglishAlphabet(line[0]))
    {
        for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(line[0]) - 'A']; *modules; ++modules)
        {
            FFModuleBaseInfo* baseInfo = *modules;
            if (ffStrEqualsIgnCase(line, baseInfo->name))
            {
                uint8_t optionBuf[FF_OPTION_MAX_SIZE];
                baseInfo->initOptions(optionBuf);
                if (__builtin_expect(jsonDoc != NULL, false))
                    fn(baseInfo, optionBuf, jsonDoc);
                else
                    baseInfo->printModule(optionBuf);
                baseInfo->destroyOptions(optionBuf);
                return;
            }
        }
    }

    ffPrintError(line, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "<no implementation provided>");
}

void ffPrintCommandOption(FFdata* data, yyjson_mut_doc* jsonDoc)
{
    //Parse the structure and call the modules
    int32_t thres = instance.config.display.stat;
    uint32_t startIndex = 0;
    while (startIndex < data->structure.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&data->structure, startIndex, ':');
        data->structure.chars[colonIndex] = '\0';

        double ms = 0;
        if(thres >= 0)
            ms = ffTimeGetTick();

        parseStructureCommand(data->structure.chars + startIndex, genJsonResult, jsonDoc);

        if(thres >= 0)
        {
            ms = ffTimeGetTick() - ms;

            if (jsonDoc)
            {
                yyjson_mut_val* moduleJson = yyjson_mut_arr_get_last(jsonDoc->root);
                yyjson_mut_obj_add_real(jsonDoc, moduleJson, "stat", ms);
            }
            else
            {
                char str[64];
                int len = snprintf(str, sizeof str, "%.3fms", ms);
                if (thres > 0)
                    snprintf(str, sizeof str, "\e[%sm%.3fms\e[m", (ms <= thres ? FF_COLOR_FG_GREEN : ms <= 2 * thres ? FF_COLOR_FG_YELLOW : FF_COLOR_FG_RED), ms);
                printf("\e7\e[1A\e[9999999C\e[%dD%s\e8", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
            }
        }

        #if defined(_WIN32)
            if (!jsonDoc && !instance.config.display.noBuffer) fflush(stdout);
        #endif

        startIndex = colonIndex + 1;
    }
}

void ffMigrateCommandOptionToJsonc(FFdata* data, yyjson_mut_doc* jsonDoc)
{
    //If we don't have a custom structure, use the default one
    if(data->structure.length == 0)
        ffStrbufAppendS(&data->structure, FASTFETCH_DATATEXT_STRUCTURE); // Cannot use `ffStrbufSetStatic` here because we will modify the string

    //Parse the structure and call the modules
    uint32_t startIndex = 0;
    while (startIndex < data->structure.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&data->structure, startIndex, ':');
        data->structure.chars[colonIndex] = '\0';

        parseStructureCommand(data->structure.chars + startIndex, genJsonConfig, jsonDoc);

        startIndex = colonIndex + 1;
    }
}
