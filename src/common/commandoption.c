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

bool ffParseModuleOptions(const char* key, const char* value)
{
    if (!ffStrStartsWith(key, "--") || !ffCharIsEnglishAlphabet(key[2])) return false;

    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(key[2]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        if (baseInfo->parseCommandOptions(baseInfo, key, value)) return true;
    }
    return false;
}

void ffPrepareCommandOption(FFdata* data)
{
    FFOptionsModules* const options = &instance.config.modules;
    //If we don't have a custom structure, use the default one
    if(data->structure.length == 0)
        ffStrbufAppendS(&data->structure, FASTFETCH_DATATEXT_STRUCTURE); // Cannot use `ffStrbufSetStatic` here because we will modify the string

    if(ffStrbufContainIgnCaseS(&data->structure, FF_CPUUSAGE_MODULE_NAME))
        ffPrepareCPUUsage();

    if(ffStrbufContainIgnCaseS(&data->structure, FF_DISKIO_MODULE_NAME))
        ffPrepareDiskIO(&options->diskIo);

    if(ffStrbufContainIgnCaseS(&data->structure, FF_NETIO_MODULE_NAME))
        ffPrepareNetIO(&options->netIo);

    if(instance.config.general.multithreading)
    {
        if(ffStrbufContainIgnCaseS(&data->structure, FF_PUBLICIP_MODULE_NAME))
            ffPreparePublicIp(&options->publicIP);

        if(ffStrbufContainIgnCaseS(&data->structure, FF_WEATHER_MODULE_NAME))
            ffPrepareWeather(&options->weather);
    }
}

static void genJsonConfig(FFModuleBaseInfo* baseInfo, yyjson_mut_doc* doc)
{
    yyjson_mut_val* modules = yyjson_mut_obj_get(doc->root, "modules");
    if (!modules)
        modules = yyjson_mut_obj_add_arr(doc, doc->root, "modules");

    yyjson_mut_val* module = yyjson_mut_obj(doc);
    FF_STRBUF_AUTO_DESTROY type = ffStrbufCreateS(baseInfo->name);
    ffStrbufLowerCase(&type);
    yyjson_mut_obj_add_strbuf(doc, module, "type", &type);

    if (baseInfo->generateJsonConfig)
        baseInfo->generateJsonConfig(baseInfo, doc, module);

    if (yyjson_mut_obj_size(module) > 1)
        yyjson_mut_arr_add_val(modules, module);
    else
        yyjson_mut_arr_add_strbuf(doc, modules, &type);
}

static void genJsonResult(FFModuleBaseInfo* baseInfo, yyjson_mut_doc* doc)
{
    yyjson_mut_val* module = yyjson_mut_arr_add_obj(doc, doc->root);
    yyjson_mut_obj_add_str(doc, module, "type", baseInfo->name);
    if (baseInfo->generateJsonResult)
        baseInfo->generateJsonResult(baseInfo, doc, module);
    else
        yyjson_mut_obj_add_str(doc, module, "error", "Unsupported for JSON format");
}

static void parseStructureCommand(
    const char* line,
    void (*fn)(FFModuleBaseInfo *baseInfo, yyjson_mut_doc* jsonDoc),
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
                if (__builtin_expect(jsonDoc != NULL, false))
                    fn(baseInfo, jsonDoc);
                else
                    baseInfo->printModule(baseInfo);
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
                printf("\e[s\e[1A\e[9999999C\e[%dD%s\e[u", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
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
