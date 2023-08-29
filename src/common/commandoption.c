#include "commandoption.h"
#include "common/printing.h"
#include "common/time.h"
#include "fastfetch_datatext.h"
#include "modules/modules.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <inttypes.h>

static inline yyjson_mut_val* genJson(FFModuleBaseInfo* baseInfo)
{
    yyjson_mut_doc* doc = instance.state.resultDoc;
    if (__builtin_expect(!doc, true)) return NULL;

    yyjson_mut_val* module = yyjson_mut_arr_add_obj(doc, doc->root);
    yyjson_mut_obj_add_str(doc, module, "type", baseInfo->name);
    if (baseInfo->generateJson)
        baseInfo->generateJson(baseInfo, doc, module);
    else
        yyjson_mut_obj_add_str(doc, module, "error", "Unsupported for JSON format");
    return module;
}

bool ffParseModuleCommand(const char* type)
{
    if(!isalpha(type[0])) return false;

    for (FFModuleBaseInfo** modules = ffModuleInfos[toupper(type[0]) - 'A']; *modules; ++modules)
    {
        FFModuleBaseInfo* baseInfo = *modules;
        if (ffStrEqualsIgnCase(type, baseInfo->name))
        {
            if (!genJson(baseInfo))
                baseInfo->printModule(baseInfo);
            return true;
        }
    }
    return false;
}

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
    //If we don't have a custom structure, use the default one
    if(data->structure.length == 0)
        ffStrbufAppendS(&data->structure, FASTFETCH_DATATEXT_STRUCTURE);

    if(ffStrbufContainIgnCaseS(&data->structure, FF_CPUUSAGE_MODULE_NAME))
        ffPrepareCPUUsage();

    if(instance.config.multithreading)
    {
        if(ffStrbufContainIgnCaseS(&data->structure, FF_PUBLICIP_MODULE_NAME))
            ffPreparePublicIp(&instance.config.publicIP);

        if(ffStrbufContainIgnCaseS(&data->structure, FF_WEATHER_MODULE_NAME))
            ffPrepareWeather(&instance.config.weather);
    }
}

static void parseStructureCommand(const char* line, FFlist* customValues)
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
            ffPrintCustom(&options);
            return;
        }
    }

    if(!ffParseModuleCommand(line))
        ffPrintErrorString(line, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "<no implementation provided>");
}

void ffPrintCommandOption(FFdata* data)
{
    //Parse the structure and call the modules
    uint32_t startIndex = 0;
    while (startIndex < data->structure.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&data->structure, startIndex, ':');
        data->structure.chars[colonIndex] = '\0';

        uint64_t ms = 0;
        if(__builtin_expect(instance.config.stat, false))
            ms = ffTimeGetTick();

        parseStructureCommand(data->structure.chars + startIndex, &data->customValues);

        if(__builtin_expect(instance.config.stat, false))
        {
            char str[32];
            int len = snprintf(str, sizeof str, "%" PRIu64 "ms", ffTimeGetTick() - ms);
            if(instance.config.pipe)
                puts(str);
            else
                printf("\033[s\033[1A\033[9999999C\033[%dD%s\033[u", len, str); // Save; Up 1; Right 9999999; Left <len>; Print <str>; Load
        }

        #if defined(_WIN32)
            if (!instance.config.noBuffer) fflush(stdout);
        #endif

        startIndex = colonIndex + 1;
    }
}
