#include "commandoption.h"
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
    if (!ffStrStartsWith(key, "--") || !isalpha(key[2])) return false;

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
    FFlist* customValues,
    void (*fn)(FFModuleBaseInfo *baseInfo, yyjson_mut_doc* jsonDoc),
    yyjson_mut_doc* jsonDoc
)
{
    // handle `--set` and `--set-keyless`
    FF_LIST_FOR_EACH(FFCustomValue, customValue, *customValues)
    {
        if (ffStrbufEqualS(&customValue->key, line))
        {
            __attribute__((__cleanup__(ffDestroyCustomOptions))) FFCustomOptions options;
            ffInitCustomOptions(&options);
            if (customValue->printKey)
                ffStrbufAppend(&options.moduleArgs.key, &customValue->key);
            ffStrbufAppend(&options.moduleArgs.outputFormat, &customValue->value);
            if (__builtin_expect(jsonDoc != NULL, false))
                fn((FFModuleBaseInfo*) &options, jsonDoc);
            else
                ffPrintCustom(&options);
            return;
        }
    }

    if(isalpha(line[0]))
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
    uint32_t startIndex = 0;
    while (startIndex < data->structure.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&data->structure, startIndex, ':');
        data->structure.chars[colonIndex] = '\0';

        uint64_t ms = 0;
        if(instance.config.display.stat)
            ms = ffTimeGetTick();

        parseStructureCommand(data->structure.chars + startIndex, &data->customValues, genJsonResult, jsonDoc);

        if(instance.config.display.stat)
        {
            ms = ffTimeGetTick() - ms;

            if (jsonDoc)
            {
                yyjson_mut_val* moduleJson = yyjson_mut_arr_get_last(jsonDoc->root);
                yyjson_mut_obj_add_uint(jsonDoc, moduleJson, "stat", ms);
            }
            else
            {
                char str[32];
                int len = snprintf(str, sizeof str, "%" PRIu64 "ms", ms);
                if(instance.config.display.pipe)
                    puts(str);
                else
                    printf("\033[s\033[1A\033[9999999C\033[%dD%s\033[u", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
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

        parseStructureCommand(data->structure.chars + startIndex, &data->customValues, genJsonConfig, jsonDoc);

        startIndex = colonIndex + 1;
    }
}
